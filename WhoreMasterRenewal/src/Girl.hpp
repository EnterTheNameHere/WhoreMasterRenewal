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
    Girl( Girl&& ) = delete;
    Girl& operator = ( const Girl& ) = delete;
    Girl&& operator = ( Girl&& ) = delete;
    
private:
    // ID
    // First Name, Last Name
    // Traits - vector
    // Inventory - vector
    // Statistics - vector
    // Skills - vector
    // Day Job ID
    // Night Job ID
    // Money
}

} // namespace

#endif // GIRL_HPP_INCLUDED_1616
