//
//  Dg.h
//  Wires
//
//  Created by Nick Porcino on 7/19/14.
//  Copyright (c) 2014 PlanetIx. All rights reserved.
//
#pragma once

#include <unordered_map>
#include <unordered_set>
#include <typeindex>
#include <set>
#include <string>
#include <vector>

template<class K, class V>
typename std::unordered_multimap<K, V>::const_iterator
find_pair(const std::unordered_multimap<K, V>& map, const std::pair<K, V>& pair) {
    auto range = map.equal_range(pair.first);
    for (auto p = range.first; p != range.second; ++p)
        if (p->second == pair.second)
            return p;
    return map.end();
}


class Dg {
private:
    class TypedData {
    public:
        TypedData() : _type(typeid(TypedData)) { }
        virtual ~TypedData() { }
        std::type_index type() const { return _type; }
        
    protected:
        std::type_index _type;
    };
    
    template <typename T>
    class Data : public TypedData {
    public:
        Data() { _type = typeid(T); }
        Data(const T& data) : _data(data) { _type = typeid(T); }
        virtual ~Data() {}
        virtual const T& value() const { return _data; }
        virtual void setValue(const T& i) { _data = i; }
        
    private:
        T _data;
    };
    
public:
    template <typename T> bool value(   const std::string& nodeName, const std::string& attrName, T& result);
    template <typename T> void setValue(const std::string& nodeName, const std::string& attrName, const T& value);
    
    void addNode(const std::string& nodeName);
    void addAttribute(const std::string& nodeName, const std::string& attrName);
    void connect(const std::string& from, const std::string& to);
    void connectAttribute(const std::string& fromNode, const std::string& fromAttr,
                          const std::string& toNode, const std::string& toAttr);
    
    void setEvaluator(const std::string& nodeName, const std::string& attrName, std::function<void(Dg&)> evalFn);
    int  addObserver( const std::string& nodeName, const std::string& attrName, std::function<void(Dg&)> callbackFn);
    void removeObserver(int observerId);
    
    std::set<std::string> roots() const;
    std::set<std::string> terminals() const;
    std::set<std::string> attributes(const std::string& node) const;

    std::vector<std::string> pred(const std::string& node) const;   // all immediate predecessors of this node
    std::vector<std::string> succ(const std::string& node) const;   // all immediate successors of this node
    
    void report();
    void reportInToOut(const std::string& node, int indent);
    void reportOutToIn(const std::string& node, int indent);
    void reportAttributes(const std::string& node, int indent);
    
private:
    template <typename T> bool value(const std::string& attrkey, T& result);
    
    std::string attrKey(const std::string& node, const std::string& attr) {
        return node + "_#_" + attr;
    }
    
    class AttrRecord {
    public:
        std::string                name;
        std::string                node;
        std::shared_ptr<TypedData> data;
        std::function<void(Dg&)>   evaluator;
    };
    
    class ObserverRecord {
    public:
        ObserverRecord(int id, std::function<void(Dg&)> cb) : id(id), callback(cb) {}
        int id;
        std::function<void(Dg&)>   callback;
    };
    
    // _nodes.reserve(expected_number_of_entries / _nodes.max_load_factor());
    std::unordered_set<std::string>                        _nodes;              // nodes
    std::unordered_multimap<std::string, std::string>      _connections;        // output -> input
    std::unordered_multimap<std::string, std::string>      _reverseConnections; // input -> output
    std::unordered_multimap<std::string, std::string>      _nodeAttributes;     // node -> attributes
    std::unordered_map<std::string, AttrRecord*>           _attributes;         // attribute -> record
    std::unordered_multimap<std::string, ObserverRecord*>  _observers;          // attribute -> observers
};

template <typename T>
bool Dg::value(const std::string& nodeName, const std::string& attrName, T& result) {
    std::string key = attrKey(nodeName, attrName);
    return value(key, result);
}

template <typename T>
bool Dg::value(const std::string& key, T& result) {
    auto it = _attributes.find(key);
    if (it == _attributes.end())
        return false;   // no such key
    
    auto incoming = _reverseConnections.equal_range(key);
    if (incoming.first != incoming.second) {
        it = _attributes.find(incoming.first->second);
        if (it != _attributes.end())
            return value<T>(attrKey(it->second->node, it->second->name), result);  // input is connected, return that
    }
    
    Data<T> *data = (Data<T>*) it->second->data.get();
    if (!data)
        return false;   // no data on attribute
    
    if (it->second->evaluator)
        it->second->evaluator(*this);
    
    result = data->value();
    return true;
}

template <typename T>
void Dg::setValue(const std::string& nodeName, const std::string& attrName, const T& value) {
    std::string key = attrKey(nodeName, attrName);
    auto it = _attributes.find(key);
    if (it == _attributes.end())
        return;
    
    Data<T> *data = (Data<T>*) it->second->data.get();
    if (data) {
        if (data->type() != typeid(T)) {
            // raise an error
            return;
        }
    }
    else {
        std::shared_ptr<Data<T>> dataPtr = std::shared_ptr<Data<T>>(new Data<T>());
        it->second->data = dataPtr;
        data = dataPtr.get();
    }
    
    data->setValue(value);
    
    // notify all observers of a change
    auto observers = _observers.equal_range(key);
    for (auto observer = observers.first; observer != observers.second; ++observer)
        observer->second->callback(*this);
}
