//Copyright © 2022 Charles Kerr. All rights reserved.

#include "huedata.hpp"
#include "strutil.hpp"
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <sstream>

using namespace std::string_literals;
constexpr auto hueentry_size = (32*2) + 2 + 2 +20 ;

//=================================================================================
//=======================================================================================================================
// huecolor_t  a hue color value
//=======================================================================================================================

//=======================================================================================================================
huecolor_t::huecolor_t(const std::string &value):huecolor_t() {
    auto values = strutil::parse(value,":") ;
    
    switch(values.size()){
        default:
        case 3:
            color = color | ((strutil::ston<std::uint16_t>(values[2]))&0x1f) ;
            [[fallthrough]];
        case 2:
            color = color | (((strutil::ston<std::uint16_t>(values[1]))&0x1f)<<5) ;
            [[fallthrough]];
        case 1:
            color = color | (((strutil::ston<std::uint16_t>(values[0]))&0x1f)<<10) ;
            [[fallthrough]];
        case 0:
            break;
    }
}
//=======================================================================================================================
auto huecolor_t::description() const ->std::string {
    auto red = ((color>>10)&0x1f);
    auto green = ((color>>5) &0x1f);
    auto blue = (color &0x1f) ;
    return std::to_string(red)+":"s+std::to_string(green)+":"s+std::to_string(blue);
}
//=======================================================================================================================
auto huecolor_t::empty() const ->bool {
    return (color & 0x7FF)<=1;
}
//=======================================================================================================================
auto huecolor_t::operator!=(const huecolor_t &value) const ->bool {
    return this->color != value.color;
}
//=======================================================================================================================
auto huecolor_t::operator==(const huecolor_t &value) const ->bool {
    return this->color == value.color ;
}

//=======================================================================================================================
// hueentry_t  A hue entry
//=======================================================================================================================
//=======================================================================================================================
hueentry_t::hueentry_t(const std::string &line):hueentry_t() {
    auto values = strutil::parse(line,",") ;
    if (values.size() != 33) {
        std::out_of_range("Hue entry line had incorrect number of entries.");
    }
    huename = values[0] ;
    if (huename.size() >20){
        huename.resize(20);
    }
    for (auto j=0 ; j< 32;j++){
        huecolor[j] = huecolor_t(values[j+1]);
    }
    return ;
}
//=======================================================================================================================
hueentry_t::hueentry_t(const std::vector<std::uint8_t> &data):hueentry_t(){
    if (data.size() != hueentry_size){
        throw std::runtime_error("Hue entry data is incorrect size.");
    }
    auto offset = 0 ;
    for (auto j=0 ; j<32;j++){
        std::copy(data.data()+offset,data.data()+offset+2,reinterpret_cast<std::uint8_t*>(&(huecolor[j].color)));
        offset += 2 ;
    }
    auto buffer = std::vector<char>(21,0);
    std::copy(data.data()+offset+4,data.data()+offset+4+20,reinterpret_cast<char*>(buffer.data()));
    for (auto j = 0; j < 20; j++) {
          if (((buffer[j] < 32) || (buffer[j] == 44) || (buffer[j] > 127)) && (buffer[j]!=0)) {
                buffer[j] = 45;
          }
    }
    huename = buffer.data() ;
    huename = strutil::trim(huename);
    // We need to ensure this doesn't have ",", or "\n", or "\r"
    for (auto& entry : huename) {
          if (entry == ',') {
                entry = '_';
          }
          else if (entry == '\n') {
                entry = '_';
          }
          else if (entry == '\r') {
                entry = '_';
          }
    }
}
//=======================================================================================================================
auto hueentry_t::data() const ->std::vector<std::uint8_t> {
    auto buffer = std::vector<std::uint8_t>(hueentry_size,0) ;
    auto offset = 0 ;
    for (auto j=0 ; j<32;j++){
        std::copy(reinterpret_cast<const std::uint8_t*>(&(huecolor[j].color)),reinterpret_cast<const std::uint8_t*>(&(huecolor[j].color))+2,buffer.data()+offset);
        offset += 2 ;
    }
    // Table start
    std::copy(reinterpret_cast<const std::uint8_t*>(&(huecolor[0].color)),reinterpret_cast<const std::uint8_t*>(&(huecolor[0].color))+2,buffer.data()+offset);
    offset +=2 ;
    // Table end
    std::copy(reinterpret_cast<const std::uint8_t*>(&(huecolor[31].color)),reinterpret_cast<const std::uint8_t*>(&(huecolor[31].color))+2,buffer.data()+offset);
    offset +=2 ;

    // Name
    auto nbuffer = std::vector<char>(20,0) ;
    auto cpysize = std::min(huename.size(),static_cast<size_t>(20)) ;
    std::copy(huename.c_str(),huename.c_str()+cpysize,nbuffer.data());
    std::copy(nbuffer.data(),nbuffer.data()+nbuffer.size(),buffer.data()+offset );
    
    return buffer ;
}
//=======================================================================================================================
auto hueentry_t::description() const ->std::string {
    std::stringstream buffer ;
    buffer << huename;
    for (const auto &entry:huecolor){
        buffer<<","<<entry.description();
    }
    return buffer.str() ;
}
//=======================================================================================================================
auto hueentry_t::empty() const ->bool {
    auto rvalue = false ;
    if (huename.empty()){
        rvalue = true ;
        for (const auto &entry:huecolor){
            if (!entry.empty()){
                rvalue = false ;
                break;
            }
        }
    }
    return rvalue ;
}

//=======================================================================================================================
auto hueentry_t::name() const ->const std::string & {
    return huename;
}
//=======================================================================================================================
auto hueentry_t::name() ->std::string& {
    return huename;
}
//=======================================================================================================================
auto hueentry_t::operator[](int index) const ->const huecolor_t& {
    return huecolor.at(index) ;
}
//=======================================================================================================================
auto hueentry_t::operator[](int index) ->huecolor_t& {
    return huecolor.at(index) ;
}
    
//=======================================================================================================================
auto hueentry_t::operator==(const hueentry_t& value) const ->bool {
    auto rvalue = false ;
    if (this->huename == value.huename){
        rvalue = true ;
        for (auto j=0 ; j<32 ; j++){
            if ((huecolor[j].color &0x7FFF) != (value.huecolor[j].color&0x7fff)){
                rvalue = false ;
                break;
            }
        }
    }
    return rvalue ;
}
//=======================================================================================================================
auto hueentry_t::operator!=(const hueentry_t& value) const ->bool {
    return !this->operator==(value);
}


//=======================================================================================================================
// huestorage_t
//=======================================================================================================================
const std::string huestorage_t::text_header="hueid,name,color0,color1,color2,color3,color4,color5,color6,color7,color8,color9,color10,color11,color12,color13,color14,color15,color16,color17,color18,color19,color20,color21,color22,color23,color24,color25,color26,color27,color28,color29,color30,color31" ;

//=======================================================================================================================
huestorage_t::huestorage_t(const std::filesystem::path &huepath,std::uint32_t maxnum):huestorage_t(maxnum){
    if (!huepath.empty()){
        load(huepath);
    }
}
//=======================================================================================================================
auto huestorage_t::load(const std::filesystem::path &huepath) ->void{
    huedata.clear() ;
    huedata.reserve(huemax) ;
    if (!std::filesystem::exists(huepath)){
        throw std::runtime_error("Does not exist: "s + huepath.string());
    }
    
     auto input = std::ifstream(huepath.string(),std::ios::binary);
    if (!input.is_open()){
        throw std::runtime_error("Unable to open: "s + huepath.string());
    }
    auto databuffer = std::vector<std::uint8_t>(hueentry_size,0);
    auto hueid = size_t(0) ;
    auto header = std::uint32_t(0);
    while (input.good() && !input.eof()){
        if ((hueid&7) == 0){
            input.read(reinterpret_cast<char*>(&header),4) ; // Seek past header
        }
        input.read(reinterpret_cast<char*>(databuffer.data()),databuffer.size());
        if (input.gcount()==databuffer.size()){
            // We read it ok
            huedata.push_back(hueentry_t(databuffer));
            if (huedata.size()> huemax){
                throw std::runtime_error("Exceeds max number of hues of: "s + std::to_string(huemax));
            }
            hueid++;
       }
    }
    
}
//=======================================================================================================================
auto huestorage_t::save(const std::filesystem::path &huepath) const ->void{
    if (huedata.empty()){
        throw std::runtime_error("No hues to save.");
    }
    auto output = std::ofstream(huepath.string(),std::ios::binary) ;
    if (!output.is_open()){
        throw std::runtime_error("Unable to create: "s + huepath.string());
    }
    auto zero = std::uint32_t(0) ;
    for (size_t j = 0 ; j<huedata.size();j++){
        if ((j&7) == 0){
            output.write(reinterpret_cast<char*>(&zero),4 );
        }
        output.write(reinterpret_cast<const char*>(huedata.at(j).data().data()),huedata.at(j).data().size());
    }
}
//=======================================================================================================================
auto huestorage_t::importText(const std::filesystem::path &huepath)->void{
    auto input = std::ifstream(huepath.string()) ;
    if (!input.is_open()){
        throw std::runtime_error("Unable to open: "s + huepath.string());
    }
    auto buffer = std::vector<char>(4049,0) ;
    auto linecount = 0 ;
    while (input.good() && !input.eof()){
        input.getline(buffer.data(), 4048);
        linecount++ ;
        if (input.gcount()>0){
            // This might have the \n or not, who knows
            buffer[input.gcount()] = 0 ;
            std::string line = std::string(buffer.data(),buffer.data()+input.gcount()) ;
            line = strutil::trim(line);
            if (!line.empty()){
                auto [first,rest] = strutil::split(line, ",");
                if (strutil::lower(first) != "hueid"){
                    // Ok, so the first is the hue id, and the rest is the huedata
                    auto id = strutil::ston<std::uint32_t>(first) ;
                    if (!rest.empty()){
                        auto needed = id +1 ;
                        if (needed > huedata.size()){
                            // Ok, so we need to increase the data size, check to see if exceeds
                            huedata.resize(needed);
                            if (huedata.size()> huemax){
                                throw std::runtime_error("Exceeds max number of hues of: "s + std::to_string(huemax));
                            }
                        }
                        huedata[id]= hueentry_t(rest);
                    }
                    else {
                        throw std::runtime_error("Bad line on line number: "s+std::to_string(linecount));
                    }
                    
                }
            }
        }
    }
}
//=======================================================================================================================
auto huestorage_t::exportText(const std::filesystem::path &huepath) const ->void {
    auto output = std::ofstream(huepath.string());
    if (!output.is_open()){
        throw std::runtime_error("Unable to create: "s+huepath.string());
    }
    output << huestorage_t::text_header<<"\n" ;
    auto hueid = 0 ;
    for (const auto &entry:huedata){
        output <<std::to_string(hueid)<<","<<entry.description()<<"\n" ;
        hueid++;
    }
    
}

//=======================================================================================================================
auto huestorage_t::size() const ->size_t{
    return huedata.size() ;
}
//=======================================================================================================================
auto huestorage_t::operator[](std::uint32_t id) const ->const hueentry_t& {
    return huedata.at(id) ;
}
//=======================================================================================================================
auto huestorage_t::operator[](std::uint32_t id)  -> hueentry_t& {
    return huedata.at(id) ;
}
//=======================================================================================================================
auto huestorage_t::empty() const->bool {
    return huedata.empty() ;
}

//=======================================================================================================================
auto huestorage_t::blank() const ->std::vector<std::uint32_t> {
    auto rvalue = std::vector<std::uint32_t>() ;
    auto hueid = 0 ;
    for (const auto &entry:huedata){
        if (entry.empty()){
                rvalue.push_back(hueid);
        }
        hueid++ ;
    }
    return rvalue ;
}
//=======================================================================================================================
auto huestorage_t::unique(const huestorage_t &storage) const ->std::vector<std::uint32_t> {
    auto rvalue = std::vector<std::uint32_t>() ;
    auto hueid = 0 ;
    for (const auto &entry:storage.huedata) {
        if (!entry.empty()){
            auto match = false ;
           
            for (const auto &mentry:huedata){
                if (entry==mentry){
                    match = true ;
                    break;
                }
            }
            if (!match){
                rvalue.push_back(hueid);
            }
        }
        hueid++;
    }
    return rvalue ;
}
//=======================================================================================================================
auto huestorage_t::merge(const huestorage_t &storage)  ->void {
    auto blanks = this->blank() ;
    auto unique = this->unique(storage) ;
    if (!unique.empty()){
        if (!blanks.empty()){
            if (blanks[0] == 0){
                // We need to erase this, we dont use it for insert
                blanks.erase(blanks.begin()) ;
            }
            auto iter = blanks.begin() ;
            for (const auto &id:unique){
                if (iter != blanks.end()){
                    std::cout <<"Inserting addition id:"<<id<<" into empty id "<<*iter<<std::endl;
                    huedata[*iter] = storage[id] ;
                    iter++ ;
                }
                else {
                    auto temp = this->append(storage[id]) ;
                    std::cout <<"Expanding for "<<id<<" placed at id "<<temp<<std::endl;
                }
            }
        }
    }
    else {
        throw std::runtime_error("Nothing unique in to merge");
    }

}

//=======================================================================================================================
auto huestorage_t::append(const hueentry_t &entry)->std::uint32_t {
    if (huedata.size()>=huemax) {
        throw std::runtime_error("Adding an entry would exceed max number of hues: "s+std::to_string(huemax));
    }
    huedata.push_back(entry) ;
    return static_cast<std::uint32_t>(huedata.size()-1) ;
}
