// src/tools/tool_registry.cpp — YOU IMPLEMENT (spec §4.3).
//
// A hand-written hash table. No std::unordered_map / std::map / std::set.
// Every function below is a stub so the starter compiles. Replace each body.

#include "tools/tool_registry.h"
#include <cstdint> 
#include <string>
#include <utility>
using namespace std;

ToolRegistry::ToolRegistry() : buckets(32) {}   // TODO: create your initial buckets
ToolRegistry::~ToolRegistry() = default;   // unique_ptrs in your buckets free the tools

std::size_t ToolRegistry::hash(std::string_view key) noexcept {
std::uint64_t h = 1469598103934665603ULL;                         // "offset basis"
for (unsigned char c : key) { h ^= c; h *= 1099511628211ULL; }     // "FNV prime"
return static_cast<std::size_t>(h); 
}

bool ToolRegistry::add(std::unique_ptr<Tool> tool) {
    //If there is nothing to store, return false
    if(!tool){
        return false;
    }

    string name = tool->name();

    //If there is a duplicate, return false
    if(find(name) != nullptr){
        return false;
    }

    //If the table is full, then make it grow
    if(size_ + 1 > buckets.size()){
        vector<Bucket> biggerBucket(buckets.size() * 2);
        for(size_t b = 0; b < buckets.size(); b++){ //go through old buckets
            for(size_t t = 0; t < buckets[b].size(); t++){ //and through every tool
                string name = buckets[b][t] -> name(); //get name of tool
                size_t bucketNumber = hash(name) % biggerBucket.size(); //calculates new buclet size
                biggerBucket[bucketNumber].push_back(move(buckets[b][t])); //move the tool pointer
            }
        } 
        buckets = move(biggerBucket);
    }

    size_t bucketNumber = hash(name) % buckets.size(); //Picks the bucket to be used
    buckets[bucketNumber].push_back(move(tool));
    size_++;
    return true;  
}

Tool* ToolRegistry::find(std::string_view name) const {
    size_t bucketNumber = hash(name) % buckets.size(); //Picks the bucket to be used
    for (size_t i = 0; i < buckets[bucketNumber].size(); i++){
        if(buckets[bucketNumber][i] -> name() == name){
            return buckets[bucketNumber][i].get();
        }
    }
    return nullptr;   
}

std::size_t ToolRegistry::size() const noexcept {
    
    return size_;
}

void ToolRegistry::for_each(const std::function<void(const Tool&)>& fn) const {
    for(size_t b = 0; b < buckets.size(); b++){ //go through old buckets
            for(size_t t = 0; t < buckets[b].size(); t++){ //and through every tool
                fn(*buckets[b][t]);
            }
        } 
}

std::size_t ToolRegistry::bucket_count() const noexcept {
    return buckets.size();
}

double ToolRegistry::load_factor() const noexcept {
    double loadFactor = static_cast<double>(size_) / static_cast<double>(buckets.size());
    return loadFactor;   // TODO
}

std::size_t ToolRegistry::max_probe_length() const {
    size_t longest = 0;
    for(size_t b = 0; b < buckets.size(); b++){ //go through buckets
        if(buckets[b].size() > longest){
            longest = buckets[b].size();
        }
    } 
    return longest;
}
