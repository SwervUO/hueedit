//Copyright © 2022 Charles Kerr. All rights reserved.

#include <iostream>
#include <stdexcept>
#include <string>
#include <cstdint>
#include <cstdlib>
#include <unordered_map>
#include <vector>

#include "argument.hpp"
#include "strutil.hpp"
#include "huedata.hpp"

using namespace std::string_literals;


//================================================================================
auto determine_ids(const std::string& list) ->std::vector<std::uint32_t> {
    auto rvalue = std::vector<std::uint32_t>() ;
    auto values = strutil::parse(list,",") ;
    for (const auto &entry:values){
        auto [first,last] = strutil::split(entry,"-") ;
        if (last.empty()){
            last = first ;
        }
        
        if (!first.empty()){
            auto start = strutil::ston<std::uint32_t>(first);
            auto finish = strutil::ston<std::uint32_t>(last);
            for (std::uint32_t j=start; j<=finish;j++){
                rvalue.push_back(j);
            }
        }
    }
    return rvalue ;
}

//================================================================================
int main(int argc, const char * argv[]) {
    enum class action_t{
        merge,extract,empty,compare,create,help
    };
    const std::unordered_map<std::string,action_t> keys{
        {"merge"s,action_t::merge},{"extract"s,action_t::extract},
        {"empty"s,action_t::empty},{"compare"s,action_t::compare},
        {"create"s,action_t::create},{"help"s,action_t::help},
    };
    auto ids = std::vector<std::uint32_t>() ;
    auto action = action_t::help ;
    auto rvalue = EXIT_SUCCESS ;
    auto maxhue = std::uint32_t(3000) ;
    try {
        auto arg = argument_t(argc,argv) ;
        for (const auto &[key,value]:arg.flags){
            if (key=="maxnum"){
                maxhue = strutil::ston<std::uint32_t>(value) ;
                if (maxhue%8 != 0) {
                    maxhue += (8 - (maxhue%8)) ;
                }
            }
            else {
                auto iter = keys.find(key) ;
                if (iter !=keys.end()){
                    if (action != action_t::help){
                        throw std::runtime_error("Conflicting action flags");
                    }
                    action = iter->second ;
                    ids = determine_ids(value) ;
                }
            }
        }
        // We now know our action
        switch (action) {
            case action_t::help:{
                std::cout <<"Usage:\n";
                std::cout <<"\thueedit --merge huemulsrc huemuladdition huemuldest\n";
                std::cout <<"\t\tMerges the entries,from huemuladdition, into the huemulsrc, and saved to huemuldest.\n";
                std::cout <<"\t\t\tThis does not preserve source ids. It merges hues into blank entries\n";
                std::cout <<"\t\t\tor appends to the end.\n";
                std::cout <<"\n" ;
                std::cout <<"\thueedit --extract huemulsrc huescvfile\n";
                std::cout <<"\t\tExtracts the entries,from huemulsrt to a csv text file.\n";
                std::cout <<"\n" ;
                std::cout <<"\thueedit --empty huemulsrc\n";
                std::cout <<"\t\tPrints the hue ids that are empty.\n";
                std::cout <<"\n" ;
                std::cout <<"\thueedit --compare huemul1 huemul2\n";
                std::cout <<"\t\tPrints the hueids that are in huemul2 but not present in huemul1.\n";
                std::cout <<"\n" ;
                std::cout <<"\thueedit --create huemul huecvsfile\n";
                std::cout <<"\t\tCreates a huemul from the cvs file.\n";
                std::cout <<"\n" ;
                std::cout <<"Note:\n";
                std::cout <<"\t The color channels in the csv range from 0-31 (5 bit channels)\n";
                std::cout <<"\n";
                std::cout <<"\t--maxhue=# allows one to create hue files greater then 3000 entries.\n";
                std::cout <<"\t\t# is the largest number of hue entries supported. Remember hue id of 0\n";
                std::cout <<"\t\tis an entry!\n";
                std::cout <<std::endl;
                break;
            }
            case action_t::merge:{
                if (arg.paths.size()<3) {
                    throw std::runtime_error("Base hue mul path, Addition mul path, and Destination mul path required.");
                }
                auto base = huestorage_t(arg.paths[0],maxhue) ;
                auto addition = huestorage_t(arg.paths[1],maxhue) ;
                base.merge(addition);
                base.save(arg.paths[2]) ;
                break;
            }
            case action_t::extract:{
                if (arg.paths.size()<2) {
                    throw std::runtime_error("Hue mul path and CSV path required.");
                }
                auto hue = huestorage_t(arg.paths[0],maxhue);
                hue.exportText(arg.paths[1]);
                break;
            }
            case action_t::empty:{
                if (arg.paths.empty()){
                    throw std::runtime_error("No hue mul file specified");
                }
                auto hues = huestorage_t(arg.paths[0],maxhue) ;
                auto missing = hues.blank() ;
                std::cout <<"Empty ids available: "<<missing.size()<<std::endl;
                for (const auto &id:missing){
                    std::cout <<"\t"<<id<<std::endl;
                }
                break;
            }
            case action_t::compare:{
                if (arg.paths.size()<2) {
                    throw std::runtime_error("Src hue mul path and Compare mul path required.");
                }

                auto huesrc = huestorage_t(arg.paths[0],maxhue);
                auto huecmp = huestorage_t(arg.paths[1],maxhue);
                auto unique = huesrc.unique(huecmp);
                std::cout <<"Unique ids in "<<arg.paths[1].filename().string()<<": "<<unique.size()<<"\n";
                for (const auto &entry:unique){
                    std::cout <<"\t"<<entry<<std::endl;
                }
                break;
            }
            case action_t::create:{
                if (arg.paths.size()<2) {
                    throw std::runtime_error("Hue mul path and CSV path required.");
                }
                auto hues = huestorage_t(maxhue) ;
                hues.importText(arg.paths[1]);
                hues.save(arg.paths[0]);
                break;
            }
        }
    }
    catch (const std::exception &e){
        std::cerr <<e.what()<<std::endl;
        rvalue = EXIT_FAILURE;
    }
    return rvalue ;
}
