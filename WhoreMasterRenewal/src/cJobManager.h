/*
 * Copyright 2009, 2010, The Pink Petal Development Team.
 * The Pink Petal Devloment Team are defined as the game's coders
 * who meet on http://pinkpetal.co.cc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef CJOBMANAGER_H_INCLUDED_1525
#define CJOBMANAGER_H_INCLUDED_1525
#pragma once

#include "Constants.h"

#include <string>
#include <vector>

namespace WhoreMasterRenewal
{

class Girl;
class Brothel;
struct sGang;
struct sCustomer;

//mainly a list of functions
class cJobManager
{
public:
    //static std::vector<sJobBase *> job_list; - Changed until it is working - necro
    // using an array of function pointers
//  WorkJobF JobFunctions[NUM_JOBS];
    bool ( *JobFunctions[NUM_JOBS] )( Girl*, Brothel*, int, std::string& );
    
    std::string JobName[NUM_JOBS];  // short descriptive name of job
    std::string JobDescription[NUM_JOBS];  // longer description of job
    std::string JobFilterName[NUMJOBTYPES];  // short descriptive name of job filter
    std::string JobFilterDescription[NUMJOBTYPES];  // longer description of job filter
    unsigned int JobFilterIndex[NUMJOBTYPES + 1]; // starting job index # for job filter
    std::string JobDescriptionCount( int job_id, int brothel_id, bool day = true ); // return a job description along with a count of how many girls are on it
    bool HandleSpecialJobs( int TargetBrothel, Girl* Girl, int JobID, int OldJobID, bool DayOrNight ); // check for and handle special job assignments
    
    void Setup();
    
    // - Misc
    static bool WorkVoid( Girl* girl, Brothel* brothel, int DayNight, std::string& summary ); // used for jobs that are not yet implemented
    
    // - General
    static bool WorkFreetime( Girl* girl, Brothel* brothel, int DayNight, std::string& summary ); // resting
    static bool AddictBuysDrugs( std::string Addiction, std::string Drug, Girl* girl, Brothel* brothel, int DayNight );
    
    static bool WorkTraining( Girl* girl, Brothel* brothel, int DayNight, std::string& summary );
    static bool WorkCleaning( Girl* girl, Brothel* brothel, int DayNight, std::string& summary );
    static bool WorkSecurity( Girl* girl, Brothel* brothel, int DayNight, std::string& summary );
    static bool WorkMatron( Girl* girl, Brothel* brothel, int DayNight, std::string& summary );
    static bool WorkAdvertising( Girl* girl, Brothel* brothel, int DayNight, std::string& summary );
    static bool WorkTorturer( Girl* girl, Brothel* brothel, int DayNight, std::string& summary );
    static bool WorkExploreCatacombs( Girl* girl, Brothel* brothel, int DayNight, std::string& summary );
    static bool WorkBeastCapture( Girl* girl, Brothel* brothel, int DayNight, std::string& summary );
    static bool WorkBeastCare( Girl* girl, Brothel* brothel, int DayNight, std::string& summary );
    
    // - Bar
    static bool WorkBar( Girl* girl, Brothel* brothel, int DayNight, std::string& summary );
    
    // - Gambling Hall
    static bool WorkHall( Girl* girl, Brothel* brothel, int DayNight, std::string& summary );
    static bool WorkShow( Girl* girl, Brothel* brothel, int DayNight, std::string& summary );
    
    // - Brothel
    static bool WorkWhore( Girl* girl, Brothel* brothel, int DayNight, std::string& summary );
    static bool WorkBrothelStripper( Girl* girl, Brothel* brothel, int DayNight, std::string& summary );
    static bool WorkBrothelMasseuse( Girl* girl, Brothel* brothel, int DayNight, std::string& summary );
    
    // - Movie Studio
    static bool WorkFluffer( Girl* girl, Brothel* brothel, int DayNight, std::string& summary );
    
    // - stuff that does processing for jobs
    
    // MYR: New code for security.  All the old code is still here, commented out.
    static bool work_related_violence( Girl*, int, bool );
    static int guard_coverage( std::vector<sGang*>* v = nullptr );
    static bool security_stops_rape( Girl* girl, sGang* enemy_gang, int day_night );
    //static bool gang_stops_rape(Girl* girl, sGang *gang, int chance, int day_night);
    static bool gang_stops_rape( Girl* girl, std::vector<sGang*> gangs_guarding, sGang* enemy_gang, int coverage, int day_night );
    //static bool girl_fights_rape(Girl*, int);
    static bool girl_fights_rape( Girl* girl, sGang* enemy_gang, int day_night );
    static void customer_rape( Girl* );
    static std::string GetGirlAttackedString();
    
    
    static bool Preprocessing( int action, Girl* girl, Brothel* brothel, int DayNight, std::string& summary, std::string& message );
    static void GetMiscCustomer( Brothel* brothel, sCustomer& cust );
    
    bool work_show( Girl* girl, Brothel* brothel, std::string& summary, int DayNight );
    void update_film( Brothel* );
    long make_money_films();
    void save_films( std::ofstream& ofs );
    void load_films( std::ifstream& ifs );
    bool apply_job( Girl* girl, int job, int brothel_id, bool day_or_night, std::string& message );
    int get_num_on_job( int job, int brothel_id, bool day_or_night );
    static bool is_sex_type_allowed( unsigned int sex_type, Brothel* brothel );
#ifndef _DEBUG
    static void free();
#else
    static void freeJobs();
#endif
    
    //helpers
    static std::vector<Girl*> girls_on_job( Brothel* brothel, u_int job_wanted, int day_or_night );
    //need a function for seeing if there is a girl working on a job
    bool is_job_employed( Brothel* brothel, u_int job_wanted, int day_or_night );
    static void get_training_set( std::vector<Girl*>& v, std::vector<Girl*>& set );
    static void do_training( Brothel* brothel, int DayNight );
    static void do_training_set( std::vector<Girl*> girls, int day_night );
    static void do_solo_training( Girl* girl, int DayNight );
    static void do_advertising( Brothel* brothel );
    ~cJobManager();
    
    bool is_job_Paid_Player( u_int Job );                   //  WD: Test for all jobs paid by player
};

} // namespace WhoreMasterRenewal

#endif // CJOBMANAGER_H_INCLUDED_1525
