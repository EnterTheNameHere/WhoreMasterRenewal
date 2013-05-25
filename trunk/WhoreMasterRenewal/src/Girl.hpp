#ifndef GIRL_HPP_INCLUDED_1616
#define GIRL_HPP_INCLUDED_1616
#pragma once

namespace WhoreMasterRenewal
{

class Girl
{
public:
    Girl();
    virtual ~Girl();
    
    Girl( const Girl& ) = delete;
    Girl& operator= ( const Girl& ) = delete;
    
private:
    
}

} // namespace

#endif // GIRL_HPP_INCLUDED_1616
