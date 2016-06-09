//
//  Wires.cpp
//  Wires
//
//  Created by Nick Porcino on 2014 05/2.
//  Copyright (c) 2014 PlanetIx. All rights reserved.
//

#include "Wires.h"
//#include "Dg.h"
#include "SpoDg.h"

#include "LabText/TextScanner.h"
#include "LabText/TextScanner.hpp"
#include <string>
#include <vector>
#include <mutex>
#include <map>
#include <set>
#include <iostream>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <stdlib.h>


/*

I had the insane idea of making an ASCII art programming language because I'm always making them for documentation, and I thought, "hey I could parse these".

Under the hood, the execution engine looks like this:

Dg fsm;
fsm.addNode("ping");
fsm.addNode("pong");
fsm.addAttribute("ping", "(after 0.5 '(goto pong)");
fsm.addAttribute("pong", "(after 0.5 '(goto ping)");
fsm.addNode("main");
fsm.addAttribute("main", "state");
fsm.setValue("main", "state", string("ping"));
fsm.addObserver("main", "state", bind([](){ cout << "state changed" << endl; }));
fsm.setValue("main", "state", string("pong"));
fsm.setValue("main", "state", string("ping"));


The rule is that signal flow goes left to right and down. A plus is a join, and a paren jumps over a wire. A greater than means input. Anything with a colon is an attribute, and they bind to the node upwards of them. Connections can be made between nodes, and also between attributes (so that one attribute can drive another).

It's a ridiculous programming language but it was a lot of fun to write. Everything below can be parsed, and works. Goofy, right?

*/

using namespace std;
using namespace Wires;

string stripAttrName(const string& attr) {
    string attrName;
    const char* strend = attr.c_str() + attr.length();
    const char* colon = tsScanForCharacter(attr.c_str(), strend, ':');
    if (colon < strend) {
        attrName = attr.substr(0, colon - attr.c_str());
    }
    else
        attrName = attr;
    return attrName;
}

class CellTag {
public:
    enum class Kind { empty, node, attribute, join, bus, hwire, vwire, crossing, endpoint, skip };
    enum class NodeKind { unknown, node, attribute };
    Kind kind;
    NodeKind nodeKind;
    int x, y;
    char c;
    string uniqueName;
    string name;
    CellTag* attribOwner;
    
    CellTag(int x, int y) : kind(Kind::empty), nodeKind(NodeKind::unknown), x(x), y(y), c(' '), attribOwner(0) {}
};

#define TAGS(x,y) tags[(y)*w + (x)]

bool followWire(vector<string>& lines, CellTag** tags, CellTag* n, int x, int y, int w, int h, bool horiz,
                vector<pair<CellTag*, CellTag*>>& connectFromTo, bool verbose) {
    bool error = false;
    while (!error) {
        // must be a wire
        if (horiz)
            while ((x < w) && ((TAGS(x,y)->kind == CellTag::Kind::hwire) || (TAGS(x,y)->kind == CellTag::Kind::crossing))) {
                ++x;
            }
        else
            while ((y < h) && ((TAGS(x,y)->kind == CellTag::Kind::vwire) || (TAGS(x,y)->kind == CellTag::Kind::crossing))) {
                ++y;
            }

        // end point or branch?
        if (TAGS(x,y)->kind == CellTag::Kind::endpoint) {
            // find a node to connect to
            ++x;
            while (x < w && TAGS(x,y)->kind == CellTag::Kind::empty) {
                ++x;
            }
            if (x == w) {
                break;  // no connected node
            }
            if (TAGS(x,y)->kind != CellTag::Kind::node) {
                error = true;
                break;
            }
            if (verbose)
                std::cout << "Connection: " << n->uniqueName << " -> " << TAGS(x,y)->uniqueName << std::endl;
            
            connectFromTo.push_back(pair<CellTag*, CellTag*>(n, TAGS(x,y)));
            break;
        }

        if (TAGS(x,y)->kind == CellTag::Kind::join) {
            if ((y < h-1) && ((TAGS(x, y+1)->kind == CellTag::Kind::vwire) ||
                              (TAGS(x, y+1)->kind == CellTag::Kind::crossing) ||
                              (TAGS(x, y+1)->kind == CellTag::Kind::join))) {
                error = followWire(lines, tags, n, x, y+1, w, h, false, connectFromTo, verbose);
            }
            if ((x < w-1) && ((TAGS(x+1, y)->kind == CellTag::Kind::hwire) ||
                              (TAGS(x+1, y)->kind == CellTag::Kind::crossing) ||
                              (TAGS(x+1, y)->kind == CellTag::Kind::join))) {
                error = followWire(lines, tags, n, x+1, y, w, h, true, connectFromTo, verbose);
            }
            break;
        }
    }
    return error;
}

string uniqueName(string name, map<string, CellTag*>& names) {
    auto n = names.find(name);
    if (n == names.end())
        return name;

    int test = 1;
    do {
        char buff[256];
        sprintf(buff, "%s _#%d", name.c_str(), test);
        string testName(buff);
        n = names.find(testName);
        if (n == names.end())
            return testName;
        ++test;
    } while(true);
}

void parse(const string& input, bool verbose) {
    map<string, CellTag*> nodes;
    vector<string> lines = TextScanner::Split(input, "\n");
    vector<pair<CellTag*, CellTag*>> connectFromTo;
    vector<pair<string, string>> attributes;

    int h = (int) lines.size();
    int w = 0;
    // how much space to allocate?
    for (auto str : lines) {
        int l = (int) str.length();
        if (l > w)
            w = l;
    }
    // make a tag grid
    CellTag** tags = (CellTag**) malloc(sizeof(CellTag*) * w * h);
    for (int x = 0; x < w; ++x)
        for (int y = 0; y < h; ++y)
            TAGS(x,y) = new CellTag(x, y);

    // classify the cells
    for (int line = 0; line < h; ++line) {
        int len = (int) lines[line].length();
        for (int x = 0; x < len; ++ x) {
            char c = lines[line][x];
            TAGS(x,line)->c = c;
            // bail out on a comment line
            if (x < len - 2 && c == '/' && lines[line][x+1] == '/')
                break;
            switch (c) {
                case '-': TAGS(x,line)->kind = CellTag::Kind::hwire; break;
                case '>': TAGS(x,line)->kind = CellTag::Kind::endpoint; break;
                case '|': TAGS(x,line)->kind = CellTag::Kind::vwire; break;
                case ')': case '(': TAGS(x,line)->kind = CellTag::Kind::crossing; break;
                case '\\': TAGS(x,line)->kind = CellTag::Kind::bus; break;
                case '+': TAGS(x,line)->kind = CellTag::Kind::join; break;
                case ' ': TAGS(x,line)->kind = CellTag::Kind::empty; break;
                case '\'':
                    do {
                        ++x;
                        if (x == len)
                            break;
                        TAGS(x,line)->kind = CellTag::Kind::node;
                        TAGS(x, line)->c = lines[line][x];
                    } while (lines[line][x] != '\'');
                    ++x;
                    break;
                case '\"':
                    do {
                        ++x;
                        if (x == len)
                            break;
                        TAGS(x,line)->kind = CellTag::Kind::node;
                        TAGS(x, line)->c = lines[line][x];
                    } while (lines[line][x] != '\"');
                    ++x;
                    break;
                default:
                    TAGS(x,line)->kind = CellTag::Kind::node;
                    if (c == '[')  {
                        do {
                            if (c == '\'')
                                do {
                                    ++x;
                                    if (x == len)
                                        break;
                                    TAGS(x, line)->kind = CellTag::Kind::node;
                                    c = lines[line][x];
                                    TAGS(x, line)->c = lines[line][x];
                                    if (c == '\'') {
                                        ++x;
                                        break;
                                    }
                                } while (true);
                            if (c == '"')
                                do {
                                    ++x;
                                    if (x == len)
                                        break;
                                    TAGS(x, line)->kind = CellTag::Kind::node;
                                    c = lines[line][x];
                                    TAGS(x, line)->c = lines[line][x];
                                    if (c == '\"') {
                                        ++x;
                                        break;
                                    }
                                } while (true);
                            ++x;
                            if (x >= len)
                                break;
                            c = lines[line][x];
                            TAGS(x,line)->kind = CellTag::Kind::node;
                            TAGS(x, line)->c = c;
                        } while (c != ']');
                    }
                    else {
                        do {
                            if (c == '\'')
                                do {
                                    ++x;
                                    if (x == len)
                                        break;
                                    TAGS(x, line)->kind = CellTag::Kind::node;
                                    c = lines[line][x];
                                    TAGS(x, line)->c = lines[line][x];
                                    if (c == '\'') {
                                        ++x;
                                        break;
                                    }
                                } while (true);
                            if (c == '"')
                                do {
                                    ++x;
                                    if (x == len)
                                        break;
                                    TAGS(x, line)->kind = CellTag::Kind::node;
                                    c = lines[line][x];
                                    TAGS(x, line)->c = lines[line][x];
                                    if (c == '\"') {
                                        ++x;
                                        break;
                                    }
                                } while (true);
                            ++x;
                            if (x >= len)
                                break;
                            c = lines[line][x];
                            if (c == ' ' || c == ')' || c == '-' || c == '|' || c == '>' || c == '\\' || c == '+')
                                break;
                            TAGS(x, line)->kind = CellTag::Kind::node;
                            TAGS(x, line)->c = c;
                        } while (true);
                    }
                    break;
            }
        }
    }
    // find the nodes
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < lines[y].length(); ++x) {
            CellTag* tag = TAGS(x,y);
            if (tag->kind == CellTag::Kind::node) {
                string nodeName;
                while (TAGS(x,y)->kind == CellTag::Kind::node) {
                    nodeName += TAGS(x,y)->c;
                    if (x+1 == lines[y].length())
                        break;
                    if (TAGS(x+1,y)->kind != CellTag::Kind::node)
                        break;
                    x += 1;
                }
                tag->uniqueName = uniqueName(nodeName, nodes);
                tag->name = nodeName;
                nodes[tag->uniqueName] = tag;
            }
        }


    // follow the wires out of the nodes
    bool error = false;
    for (auto n : nodes) {
        int x = n.second->x;
        int y = n.second->y;
        // find the end of the node
        while ((x < w) && (TAGS(x,y)->kind == CellTag::Kind::node))
            ++x;
        if (x == w)
            continue;
        // skip space
        while ((x < w) && (TAGS(x,y)->kind == CellTag::Kind::empty)) {
            ++x;
        }
        if (x == w)
            continue;  // no wire coming out

        if ((TAGS(x,y)->kind != CellTag::Kind::hwire) && (TAGS(x, y)->kind != CellTag::Kind::crossing))
            continue;  // no wire coming out

        error = followWire(lines, tags, n.second, x, y, w, h, true, connectFromTo, verbose);
    }


    // classify the nodes as attributes or nodes
    for (auto& n : nodes) {
        if (!n.second->uniqueName.length()) {
            n.second->nodeKind = CellTag::NodeKind::unknown;
        }
        else if (n.second->uniqueName[0] == '[') {
            n.second->nodeKind = CellTag::NodeKind::node;
        }
        else if (n.second->uniqueName.find(':') != string::npos) {
            n.second->nodeKind = CellTag::NodeKind::attribute;
        }
        else {
            n.second->nodeKind = CellTag::NodeKind::node;
        }

        // stripe the run for attribute detection
        int x = n.second->x;
        int y = n.second->y;
        CellTag::NodeKind kind = n.second->nodeKind;
        for (int i = 0; i < n.second->uniqueName.length(); ++i) {
            TAGS(x + i, y)->nodeKind = kind;
        }
    }

    
    // add the attributes
    for (auto& n : nodes)
        if (n.second->nodeKind == CellTag::NodeKind::attribute) {
            
            int x = n.second->x;
            int y = n.second->y;
            
            if (y == 0) {
                if (verbose)
                    cerr << "Attribute " << n.second->uniqueName << " has nothing above it" << endl;
                continue;   // an attribute at the top is joined to nothing
            }
            string::size_type pos = n.second->uniqueName.find(':');
            if (pos == string::npos) {
                if (verbose)
                    cerr << "Attibute " << n.second->uniqueName << " contains no :" << endl; // this would because of an error in a previous step
                continue;
            }
            // start one above the discovered colon
            x += pos;
            --y;
            
            // what node is this an attribute of? Follow the : attribute stack upwards to a node starting with [, or a node without a :
            do {
                CellTag::NodeKind kind = TAGS(x,y)->nodeKind;
                if (kind == CellTag::NodeKind::node) {
                    int testx = x;
                    while (!TAGS(testx,y)->uniqueName.length()) {
                        --testx; // scan back for the beginning of the node where the name is stored.
                        if (testx < 0)
                            break;
                    }
                    
                    if (testx >= 0) {
                        n.second->attribOwner = TAGS(testx,y);
                        attributes.push_back(pair<string,string>(TAGS(testx,y)->uniqueName, n.second->uniqueName));
                    }
                    break;
                }
                if (kind == CellTag::NodeKind::attribute) {
                    --y;        // scan up through the attribute stack
                    continue;
                }
                if (verbose)
                    cout << "Free standing attribute: " << n.second->uniqueName << endl;
                break;
            } while (y > 0);
        }
    

    if (verbose) {
        cout << "-----------------------\nUNTAGGED\n-----------------------\n";
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                switch (TAGS(x, y)->kind) {
                    case CellTag::Kind::empty: cout << "."; break;
                    case CellTag::Kind::node: cout << "N"; break;
                    case CellTag::Kind::attribute: cout << "A"; break;
                    case CellTag::Kind::join: cout << "+"; break;
                    case CellTag::Kind::bus: cout << "/"; break;
                    case CellTag::Kind::hwire: cout << "-"; break;
                    case CellTag::Kind::vwire: cout << "|"; break;
                    case CellTag::Kind::crossing: cout << ")"; break;
                    case CellTag::Kind::endpoint: cout << ">"; break;
                    case CellTag::Kind::skip: cout << "~"; break;
                }
            }
            cout << endl;
        }
    
        cout << "-----------------------\nTAGGED\n-----------------------\n";
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                switch (TAGS(x, y)->nodeKind) {
                    case CellTag::NodeKind::attribute: cout << "A"; break;
                    case CellTag::NodeKind::node: cout << "N"; break;
                    case CellTag::NodeKind::unknown: cout << "."; break;
                }
            }
            cout << endl;
        }

        cout << "-----------------------\nNODES\n-----------------------\n";
        for (auto i : nodes) {
            switch (i.second->nodeKind) {
                case CellTag::NodeKind::attribute: cout << "attr: "; break;
                case CellTag::NodeKind::node: cout << "node: "; break;
                case CellTag::NodeKind::unknown: cout << "unknown: "; break;
            }
            cout << i.first << endl;
        }
    }
    
    //////////////////////////////////////////////////////////////////
    // create the Dg
    //
    cout << "===== Dg =======" << endl;
    
    Dg dg;
    
    // add the nodes
    for (auto& n : nodes)
        if (n.second->nodeKind == CellTag::NodeKind::node)
            dg.addNode(n.second->uniqueName);
    
    // connect the nodes
    for (auto& c : connectFromTo) {
        if (!c.first->attribOwner && !c.second->attribOwner)
            dg.connect(c.first->name, c.second->name);
        else if (c.first->attribOwner && c.second->attribOwner) {
            string firstAttr = stripAttrName(c.first->name);
            string secondAttr = stripAttrName(c.second->name);
            dg.connectAttribute(c.first->attribOwner->name, firstAttr, c.second->attribOwner->name, secondAttr);
        }
        else
            cout << "Couldn't connect attribute to non-attribute" << c.first->name << " " << c.second->name << endl;
    }
    
    // add the attributes
    for (auto& a : attributes) {
        auto n = nodes[a.second];
        
        string attr = n->name;
        string attrName;
        string attrValue;
        const char* strend = attr.c_str() + attr.length();
        const char* colon = tsScanForCharacter(attr.c_str(), strend, ':');
        if (colon < strend) {
            attrName = attr.substr(0, colon - attr.c_str());
            attrValue = attr.substr(colon - attr.c_str() + 1, attr.length());
        }
        else
            attrName = attr;
        
        string strVal;
        float floatVal = 0.f;
        if (attrValue.length() > 0) {
            
            if ((*attrValue.begin() == '\'' && *attrValue.rbegin() == '\'') ||
                (*attrValue.begin() == '\"' && *attrValue.rbegin() == '\"')) {
                strVal = attrValue.substr(1, attrValue.length() - 2);
            }
            else {
                tsGetFloat(attrValue.c_str(), attrValue.c_str() + attrValue.length(), &floatVal);
            }
        }
        
        dg.addAttribute(a.first, attrName);
        if (strVal.length() > 0)
            dg.setValue(a.first, attrName, strVal);
        else
            dg.setValue(a.first, attrName, floatVal);
    }

    dg.report();
    
    //
    // end of create the DAG
    //////////////////////////////////////////////////////////////////

    // cleaning
    for (int x = 0; x < w; ++x)
        for (int y = 0; y < h; ++y)
            delete TAGS(x,y);
}
