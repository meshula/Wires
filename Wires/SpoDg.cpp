//
//  Dg.cpp
//  Wires
//
//  Created by Nick Porcino on 7/19/14.
//  Copyright (c) 2014 PlanetIx. All rights reserved.
//

#include "SpoDg.h"
#include "LabText/TextScanner.h"

#include "leveldb/db.h"
#include "leveldb/comparator.h"
#include "leveldb/write_batch.h"

#include <iostream>

typedef float M44f;
using namespace std;

namespace Wires {

    //to do, the evaluator should take a cache validity check which could be a time cache so that if the output is cached at time,
    //just return it. The recursion of the value method will take care of lazy early outs.

    void sample() {
        Dg dg;
        dg.addNode("xform1");
        dg.addAttribute("xform1", "xformOut");
        dg.addNode("xform2");
        dg.addAttribute("xform2", "xformOut");
        dg.addNode("xformFinal");
        dg.addAttribute("xformFinal", "xformL");
        dg.addAttribute("Xformfinal", "xfromR");
        dg.addAttribute("xformFinal", "xformOut");
        dg.connectAttribute("xform1", "xformOut", "xformFinal", "xformL");
        dg.connectAttribute("xform2", "xformOut", "xformFinal", "xformR");
        dg.setEvaluator("xformFinal", "xformOut", std::function<void(Dg&)>([](Dg& dg) {
            M44f l,r;
            dg.value<M44f>("xformFinal", "xformL", l);
            dg.value<M44f>("xformFinal", "xformR", r);
            dg.setValue<M44f>("xformFinal", "xformOut", l * r);
        }));
    }

    class Dg::Detail {
    public:
        Detail() {
            leveldb::Options options;
            options.create_if_missing = true;
            leveldb::Status status = leveldb::DB::Open(options, "/tmp/testdb", &db);
            if (!status.ok()) {
                cerr << "Couldn't open db" << endl;
                db = 0;
            }
        }

        ~Detail() {
            delete db;
        }

        leveldb::DB* db;
    };

    Dg::Dg() : _detail(new Detail()) {
    }

    Dg::~Dg() {
        delete _detail;
    }

    static string spoKey(const std::string& subject, const std::string& predicate, const std::string& object, int index) {
        switch (index) {
            case 0: return "~sop~" + subject + "~" + object + "~" + predicate;
            default:
            case 1: return "~spo~" + subject + "~" + predicate + "~" + object;
            case 2: return "~osp~" + object + "~" + subject + "~" + predicate;
            case 3: return "~ops~" + object + "~" + predicate + "~" + subject;
            case 4: return "~pso~" + predicate + "~" + subject + "~" + object;
            case 5: return "~pos~" + predicate + "~" + object + "~" + subject;
        }
    }

    void Dg::connect(const std::string& subject, const std::string& predicate, const std::string& object) {
        if (_detail->db) {
            // set up the sop relationship according to the principles in
            // http://www.vldb.org/pvldb/1/1453965.pdf Hexastore: sextuple indexing for
            // semantic web data management C Weiss, P Karras, A Bernstein -
            // Proceedings of the VLDB Endowment, 2008
            //
            leveldb::WriteBatch batch;
            string val = "{subject:\"" + subject + "\",predicate:\"" + predicate + "\",object:\"";
            for (int i = 0; i < 6; ++i)
                batch.Put(spoKey(subject, predicate, object, i), val);
            leveldb::Status status = _detail->db->Write(leveldb::WriteOptions(), &batch);
        }
    }

    void Dg::disconnect(const std::string& subject, const std::string& predicate, const std::string& object) {
        if (_detail->db) {
            leveldb::WriteBatch batch;
            for (int i = 0; i < 6; ++i)
                batch.Delete(spoKey(subject, predicate, object, i));
        }
    }

    void Dg::subjectsOf(const std::string& predicate) {
        string startStr = "~pso~" + predicate + "~";
        string endStr = "~pso~" + predicate + "~\xff";
        leveldb::Slice start = startStr;
        leveldb::Slice end = endStr;
        leveldb::Options options;
        leveldb::Iterator* it = _detail->db->NewIterator(leveldb::ReadOptions());
        for (it->Seek(start); it->Valid() && (options.comparator->Compare(it->key(), end) <= 0); it->Next()) {
            cout << "Key: " << it->key().ToString() << " Value: " << it->value().ToString() << endl;
        }
    }

    void Dg::query(const string& subjects, const string& predicates, const string& objects) {
        leveldb::Iterator* it = _detail->db->NewIterator(leveldb::ReadOptions());
        leveldb::Options options;

        bool findSubjects = subjects == "*";
        bool findPredicates = subjects == "*";
        bool findObjects = subjects == "*";
        if (findSubjects && findPredicates && findObjects) {
            // SPO output everything
            for (it->SeekToFirst(); it->Valid(); it->Next()) {
                cout << "Key: " << it->key().ToString() << " Value: " << it->value().ToString() << endl;
            }
        }
        else if (!findSubjects && !findPredicates && !findObjects) {
            // sop find a specific relationship
            leveldb::Slice findMe = "~sop~" + subjects + "~" + objects + "~" + predicates;
            string result;
            leveldb::Slice start = findMe;
            leveldb::Slice end = findMe;
            leveldb::Options options;
            leveldb::Iterator* it = _detail->db->NewIterator(leveldb::ReadOptions());
            for (it->Seek(start); it->Valid() && (options.comparator->Compare(it->key(), end) <= 0); it->Next()) {
                cout << "Key: " << it->key().ToString() << " Value: " << it->value().ToString() << endl;
            }
        }
        else if (!findSubjects && findPredicates && findObjects) {
            // sPO find everything subject is in
            string startStr = "~sop~" + subjects + "~";
            string endStr = "~sop~" + subjects + "~\xff";
            leveldb::Slice start = startStr;
            leveldb::Slice end = endStr;
            leveldb::Options options;
            leveldb::Iterator* it = _detail->db->NewIterator(leveldb::ReadOptions());
            for (it->Seek(start); it->Valid() && (options.comparator->Compare(it->key(), end) <= 0); it->Next()) {
                cout << "Key: " << it->key().ToString() << " Value: " << it->value().ToString() << endl;
            }
        }
        else if (findSubjects && findPredicates && !findObjects) {
            // SPo find every relationship object is in
            string startStr = "~osp~" + objects + "~";
            string endStr = "~osp~" + objects + "~\xff";
            leveldb::Slice start = startStr;
            leveldb::Slice end = endStr;
            leveldb::Options options;
            leveldb::Iterator* it = _detail->db->NewIterator(leveldb::ReadOptions());
            for (it->Seek(start); it->Valid() && (options.comparator->Compare(it->key(), end) <= 0); it->Next()) {
                cout << "Key: " << it->key().ToString() << " Value: " << it->value().ToString() << endl;
            }
        }
        else if (findSubjects && !findPredicates && findObjects) {
            // SpO find every relationship of type predicate
            string startStr = "~pso~" + predicates + "~";
            string endStr = "~pso~" + predicates + "~\xff";
            leveldb::Slice start = startStr;
            leveldb::Slice end = endStr;
            leveldb::Options options;
            leveldb::Iterator* it = _detail->db->NewIterator(leveldb::ReadOptions());
            for (it->Seek(start); it->Valid() && (options.comparator->Compare(it->key(), end) <= 0); it->Next()) {
                cout << "Key: " << it->key().ToString() << " Value: " << it->value().ToString() << endl;
            }
        }
        else if (!findSubjects && !findPredicates && findObjects) {
            // spO all objects connected to a particular s/p
            string startStr = "~spo~" + subjects + "~" + predicates + "~";
            string endStr = "~spo~" + subjects + "~" + predicates + "~\xff";
            leveldb::Slice start = startStr;
            leveldb::Slice end = endStr;
            leveldb::Options options;
            leveldb::Iterator* it = _detail->db->NewIterator(leveldb::ReadOptions());
            for (it->Seek(start); it->Valid() && (options.comparator->Compare(it->key(), end) <= 0); it->Next()) {
                cout << "Key: " << it->key().ToString() << " Value: " << it->value().ToString() << endl;
            }
        }
        else if (findSubjects && !findPredicates && !findObjects) {
            // Spo all subjects connected to a particular p/o
            string startStr = "~pos~" + predicates + "~" + objects + "~";
            string endStr = "~pos~" + predicates + "~" + objects + "~\xff";
            leveldb::Slice start = startStr;
            leveldb::Slice end = endStr;
            leveldb::Options options;
            leveldb::Iterator* it = _detail->db->NewIterator(leveldb::ReadOptions());
            for (it->Seek(start); it->Valid() && (options.comparator->Compare(it->key(), end) <= 0); it->Next()) {
                cout << "Key: " << it->key().ToString() << " Value: " << it->value().ToString() << endl;
            }
        }
        else if (findSubjects && !findPredicates && !findObjects) {
            // sPo all predicates joining a particular s/o
            string startStr = "~sop~" + subjects + "~" + objects + "~";
            string endStr = "~sop~" + subjects + "~" + objects + "~\xff";
            leveldb::Slice start = startStr;
            leveldb::Slice end = endStr;
            leveldb::Options options;
            leveldb::Iterator* it = _detail->db->NewIterator(leveldb::ReadOptions());
            for (it->Seek(start); it->Valid() && (options.comparator->Compare(it->key(), end) <= 0); it->Next()) {
                cout << "Key: " << it->key().ToString() << " Value: " << it->value().ToString() << endl;
            }
        }
    }

#if 0
    need to add a nodekey, so the hexastore has a node key, the node key has the actual name of the node

    node store
    ~key~###hash### nodename
    ~node~nodename ###hash###

    attrib store
    ~val~###hash###~attrib value
    ~to~###hash###~###hash###
    ~from~###hash###~###hash###

    roots
    terminals
#endif

    void Dg::connect(const string &from, const string &to) {
        pair<string, string> key(from, to);
        if (find_pair(_connections, key) == _connections.end()) {
            _connections.insert(key);
            _reverseConnections.insert(pair<string, string>(to, from));
        }
    }

    void Dg::connectAttribute(const string &from, const string& fromAttr, const string &to, const string& toAttr) {
        pair<string, string> key(attrKey(from, fromAttr), attrKey(to, toAttr));
        if (find_pair(_connections, key) == _connections.end()) {
            _connections.insert(key);
            _reverseConnections.insert(pair<string, string>(attrKey(to, toAttr), attrKey(from, fromAttr)));
        }
    }

    set<string> Dg::roots() const {
        set<string> result;
        for (auto n : _connections) {
            auto el = _reverseConnections.equal_range(n.first);
            if (el.first == el.second)
                result.insert(n.first);  // no reverse connections means a root
        }
        return result;
    }
    set<string> Dg::terminals() const {
        set<string> result;
        for (auto n : _reverseConnections) {
            auto el = _connections.equal_range(n.first);
            if (el.first == el.second)
                result.insert(n.first);  // no forward connections means a terminal
        }
        return result;
    }
    set<string> Dg::attributes(const string& node) const {
        set<string> result;
        auto el = _nodeAttributes.equal_range(node);
        for (auto i = el.first; i != el.second; ++i)
            result.insert(i->second);
        return result;
    }

    void Dg::addNode(const string &nodeName) {
        /// @TODO should throw if it already exists
        if (_nodes.find(nodeName) == _nodes.end())
            _nodes.insert(nodeName);
    }

    void Dg::addAttribute(const string &nodeName, const string &attrName) {
        string key = attrKey(nodeName, attrName);
        if (_attributes.find(key) != _attributes.end())
            return;

        pair<string, string> kv(nodeName, attrName);
        _nodeAttributes.insert(kv);
        AttrRecord* ar = new AttrRecord();
        ar->node = nodeName;
        ar->name = attrName;
        _attributes[key] = ar;
    }

    void Dg::setEvaluator(const std::string& nodeName, const std::string& attrName, std::function<void(Dg&)> evalFn) {
        std::string key = attrKey(nodeName, attrName);
        auto it = _attributes.find(key);
        AttrRecord *record = 0;
        if (it == _attributes.end()) {
            record = new AttrRecord();
            _attributes[key] = record;
        }
        else
            record = it->second;
        record->evaluator = evalFn;
    }

    int Dg::addObserver(const std::string &nodeName, const std::string &attrName, std::function<void (Dg &)> callbackFn) {
        static int unique = 0;
        _observers.insert(pair<string, ObserverRecord*>(attrKey(nodeName, attrName), new ObserverRecord(unique++, callbackFn)));
        return 0;
    }

    vector<string> Dg::pred(const string& node) const {
        vector<string> ret;
        auto preds = _reverseConnections.equal_range(node);
        for (auto pred = preds.first; pred != preds.second; ++pred)
            ret.push_back(pred->second);
        return ret;
    }

    vector<string> Dg::succ(const string& node) const {
        vector<string> ret;
        auto succs = _connections.equal_range(node);
        for (auto succ = succs.first; succ != succs.second; ++succ)
            ret.push_back(succ->second);
        return ret;
    }

    void Dg::reportInToOut(const string& node, int indent) {
        char buff[indent+1];
        for (int i = 0; i < indent; ++i)
            buff[i] = ' ';
        buff[indent] = '\0';
        printf("%s%s\n", buff, node.c_str());
        auto nodes = _connections.equal_range(node);
        for (auto i = nodes.first; i != nodes.second; ++i)
            reportInToOut(i->second, indent + 3);
    }

    void Dg::reportOutToIn(const string& node, int indent) {
        char buff[indent+1];
        for (int i = 0; i < indent; ++i)
            buff[i] = ' ';
        buff[indent] = '\0';
        printf("%s%s\n", buff, node.c_str());
        auto nodes = _reverseConnections.equal_range(node);
        for (auto i = nodes.first; i != nodes.second; ++i)
            reportOutToIn(i->second, indent + 3);
    }

    void Dg::reportAttributes(const string& node, int indent) {
        char buff[indent+1];
        for (int i = 0; i < indent; ++i)
            buff[i] = ' ';
        buff[indent] = '\0';
        auto nodes = _nodeAttributes.equal_range(node);
        if (nodes.first != nodes.second) {
            printf("%s%s\n", buff, node.c_str());
            for (auto i = nodes.first; i != nodes.second; ++i) {
                bool reported = true;
                auto key = attrKey(i->first, i->second);
                auto attr = _attributes.find(key);
                if (attr != _attributes.end()) {
                    AttrRecord* rec = attr->second;
                    type_index type = rec->data->type();
                    if      (type == typeid(int))    { int v;    value<int>(attr->first, v);    printf("%s   %s:%d\n", buff, i->second.c_str(), v); }
                    else if (type == typeid(float))  { float v;  value<float>(attr->first, v);  printf("%s   %s:%f\n", buff, i->second.c_str(), v); }
                    else if (type == typeid(string)) { string v; value<string>(attr->first, v); printf("%s   %s:%s\n", buff, i->second.c_str(), v.c_str()); }
                    else reported = false;
                }
                else
                    reported = false;

                if (!reported)
                    printf("%s   %s\n", buff, i->second.c_str());

                //reportAttributes(i->second, indent + 3);
            }
        }
    }

    void Dg::report() {
        printf(">> Dg nodes\n");
        for (auto n : _nodes)
            printf("   %s\n", n.c_str());

        printf("----- Dg in -> out ------------\n");
        set<string> rootVec = roots();
        for (auto root : rootVec)
            reportInToOut(root, 0);

        printf("----- Dg out -> in ------------\n");
        set<string> leafVec = terminals();
        for (auto leaf : leafVec)
            reportOutToIn(leaf, 0);

        bool titled = false;
        for (auto n : _nodes) {
            auto attrs = _nodeAttributes.equal_range(n);
            if (attrs.first != attrs.second) {
                if (!titled) {
                    printf("----- Attributes on nodes ------\n");
                    titled = true;
                }
                reportAttributes(n, 0);
            }
        }
        printf("================================\n");
    }


} // Wires
