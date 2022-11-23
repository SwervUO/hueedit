//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef huedata_hpp
#define huedata_hpp

#include <cstdint>
#include <string>
#include <array>
#include <vector>
#include <map>
#include <istream>
#include <filesystem>

//=================================================================================
/*
 3.7 HUES.MUL
 Just read in HueGroups until you hit the end of the file. Note that large chunks of this file consist of garbage--OSI admits to this (something about a bug in their old code).
 
 If you want to look at the hues, check out this.
 
 Hues are applied to an image in one of two ways. Either all gray pixels are mapped to the specified color range (resulting in a part of an image changed to that color) or all pixels are first mapped to grayscale and then the specified color range is mapped over the entire image.
 
 HueEntry
 WORD ColorTable[32];
 WORD TableStart;
 WORD TableEnd;
 CHAR Name[20];
 
 HueGroup
 DWORD Header;
 HueEntry Entries[8];
 */
//=======================================================================================================================
// huecolor_t  a hue color value
//=======================================================================================================================
//=======================================================================================================================
struct huecolor_t {
    std::uint16_t color ;
    huecolor_t(std::uint16_t value =0):color(value){}
    huecolor_t(const std::string &value) ;
    auto description() const ->std::string ;
    auto empty() const ->bool ;
    auto operator!=(const huecolor_t &value) const ->bool ;
    auto operator==(const huecolor_t &value) const ->bool ;

};

//=================================================================================
//=======================================================================================================================
// hueentry_t  A hue entry
//=======================================================================================================================
class hueentry_t {
    std::array<huecolor_t,32> huecolor ;
    std::string huename ;
public:
    hueentry_t() = default;
    // Format of line: namestring,r:g:b,...repeated 32 times
    // the rgb values are 5 bits, so go betwen 0,31
    hueentry_t(const std::string &line) ;
    
    hueentry_t(const std::vector<std::uint8_t> &data);
    
    auto data() const ->std::vector<std::uint8_t> ;
    auto description() const ->std::string ;

    auto empty() const ->bool ;
    
    auto name() const ->const std::string & ;
    auto name() ->std::string& ;
    auto operator[](int index) const ->const huecolor_t& ;
    auto operator[](int index) ->huecolor_t& ;
    
    auto operator!=(const hueentry_t& value) const ->bool ;
    auto operator==(const hueentry_t& value) const ->bool ;

};

//=======================================================================================================================
// huestorage_t  
//=======================================================================================================================
class huestorage_t {
    std::vector<hueentry_t> huedata ;
    std::uint32_t huemax ;
public:
    static const std::string text_header ;
    huestorage_t(std::uint32_t maxnum=3000):huemax(maxnum){}
    huestorage_t(const std::filesystem::path &huepath,std::uint32_t maxnum=3000) ;
    auto load(const std::filesystem::path &huepath) ->void ;
    auto save(const std::filesystem::path &huepath) const ->void;
    auto importText(const std::filesystem::path &huepath) ->void;
    auto exportText(const std::filesystem::path &huepath) const ->void;
    
    auto size() const ->size_t ;
    auto operator[](std::uint32_t id) const ->const hueentry_t& ;
    auto operator[](std::uint32_t id) -> hueentry_t& ;
    auto empty() const ->bool ;
    
    auto blank() const ->std::vector<std::uint32_t> ;
    auto unique(const huestorage_t &storage) const ->std::vector<std::uint32_t> ;
    auto merge(const huestorage_t &storage)  ->void ;
    auto append(const hueentry_t &entry) ->std::uint32_t ;
};
#endif /* huedata_hpp */
