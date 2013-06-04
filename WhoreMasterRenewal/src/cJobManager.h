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

struct sGirl;
struct sBrothel;
struct sGang;
struct sCustomer;

struct sFilm
{
    sFilm();
    
    //int total_girls;
    //int total_cost;
    float quality_multiplyer;
    std::vector<int> scene_quality;
    char time;
    int final_quality;
    bool sex_acts_flags[5];
    int total_customers;
};

//mainly a list of functions
class cJobManager
{
public:
    //static std::vector<sJobBase *> job_list; - Changed until it is working - necro
    // using an array of function pointers
//  WorkJobF JobFunctions[NUM_JOBS];
    bool ( *JobFunctions[NUM_JOBS] )( sGirl*, sBrothel*, int, std::string& );
    
    std::string JobName[NUM_JOBS];  // short descriptive name of job
    std::string JobDescription[NUM_JOBS];  // longer description of job
    std::string JobFilterName[NUMJOBTYPES];  // short descriptive name of job filter
    std::string JobFilterDescription[NUMJOBTYPES];  // longer description of job filter
    unsigned int JobFilterIndex[NUMJOBTYPES + 1]; // starting job index # for job filter
    std::string JobDescriptionCount( int job_id, int brothel_id, bool day = true ); // return a job description along with a count of how many girls are on it
    bool HandleSpecialJobs( int TargetBrothel, sGirl* Girl, int JobID, int OldJobID, bool DayOrNight ); // check for and handle special job assignments
    
    void Setup();
    
    // - Misc
    static bool WorkVoid( sGirl* girl, sBrothel* brothel, int DayNight, std::string& summary ); // used for jobs that are not yet implemented
    
    // - General
    static bool WorkFreetime( sGirl* girl, sBrothel* brothel, int DayNight, std::string& summary ); // resting
    static bool AddictBuysDrugs( std::string Addiction, std::string Drug, sGirl* girl, sBrothel* brothel, int DayNight );
    
    static bool WorkTraining( sGirl* girl, sBrothel* brothel, int DayNight, std::string& summary );
    static bool WorkCleaning( sGirl* girl, sBrothel* brothel, int DayNight, std::string& summary );
    static bool WorkSecurity( sGirl* girl, sBrothel* brothel, int DayNight, std::string& summary );
    static bool WorkMatron( sGirl* girl, sBrothel* brothel, int DayNight, std::string& summary );
    static bool WorkAdvertising( sGirl* girl, sBrothel* brothel, int DayNight, std::string& summary );
    static bool WorkTorturer( sGirl* girl, sBrothel* brothel, int DayNight, std::string& summary );
    static bool WorkExploreCatacombs( sGirl* girl, sBrothel* brothel, int DayNight, std::string& summary );
    static bool WorkBeastCapture( sGirl* girl, sBrothel* brothel, int DayNight, std::string& summary );
    static bool WorkBeastCare( sGirl* girl, sBrothel* brothel, int DayNight, std::string& summary );
    
    // - Bar
    static bool WorkBar( sGirl* girl, sBrothel* brothel, int DayNight, std::string& summary );
    
    // - Gambling Hall
    static bool WorkHall( sGirl* girl, sBrothel* brothel, int DayNight, std::string& summary );
    static bool WorkShow( sGirl* girl, sBrothel* brothel, int DayNight, std::string& summary );
    
    // - Brothel
    static bool WorkWhore( sGirl* girl, sBrothel* brothel, int DayNight, std::string& summary );
    static bool WorkBrothelStripper( sGirl* girl, sBrothel* brothel, int DayNight, std::string& summary );
    static bool WorkBrothelMasseuse( sGirl* girl, sBrothel* brothel, int DayNight, std::string& summary );
    
    // - Movie Studio
    static bool WorkFluffer( sGirl* girl, sBrothel* brothel, int DayNight, std::string& summary );
    
    // - stuff that does processing for jobs
    
    // MYR: New code for security.  All the old code is still here, commented out.
    static bool work_related_violence( sGirl*, int, bool );
    static int guard_coverage( std::vector<sGang*>* v = nullptr );
    static bool security_stops_rape( sGirl* girl, sGang* enemy_gang, int day_night );
    //static bool gang_stops_rape(sGirl* girl, sGang *gang, int chance, int day_night);
    static bool gang_stops_rape( sGirl* girl, std::vector<sGang*> gangs_guarding, sGang* enemy_gang, int coverage, int day_night );
    //static bool girl_fights_rape(sGirl*, int);
    static bool girl_fights_rape( sGirl* girl, sGang* enemy_gang, int day_night );
    static void customer_rape( sGirl* );
    static std::string GetGirlAttackedString();
    
    
    static bool Preprocessing( int action, sGirl* girl, sBrothel* brothel, int DayNight, std::string& summary, std::string& message );
    static void GetMiscCustomer( sBrothel* brothel, sCustomer& cust );
    
    bool work_show( sGirl* girl, sBrothel* brothel, std::string& summary, int DayNight );
    void update_film( sBrothel* );
    long make_money_films();
    void save_films( std::ofstream& ofs );
    void load_films( std::ifstream& ifs );
    bool apply_job( sGirl* girl, int job, int brothel_id, bool day_or_night, std::string& message );
    int get_num_on_job( int job, int brothel_id, bool day_or_night );
    static bool is_sex_type_allowed( unsigned int sex_type, sBrothel* brothel );
#ifndef _DEBUG
    static void free();
#else
    static void freeJobs();
#endif
    
    //helpers
    static std::vector<sGirl*> girls_on_job( sBrothel* brothel, u_int job_wanted, int day_or_night );
    //need a function for seeing if there is a girl working on a job
    bool is_job_employed( sBrothel* brothel, u_int job_wanted, int day_or_night );
    static void get_training_set( std::vector<sGirl*>& v, std::vector<sGirl*>& set );
    static void do_training( sBrothel* brothel, int DayNight );
    static void do_training_set( std::vector<sGirl*> girls, int day_night );
    static void do_solo_training( sGirl* girl, int DayNight );
    static void do_advertising( sBrothel* brothel );
    ~cJobManager();
    
    bool is_job_Paid_Player( u_int Job );                   //  WD: Test for all jobs paid by player
    
private:
    std::vector<sFilm*> film_list;
    // bah 2d array time for speed
    std::vector< std::vector<u_int> > job_groups;
};

#endif // CJOBMANAGER_H_INCLUDED_1525
