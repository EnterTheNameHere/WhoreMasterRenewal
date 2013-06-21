#include "BrothelManager.hpp"
#include "Girl.hpp"
#include "sConfig.h"
#include "cRng.h"
#include "cDungeon.h"
#include "cPlayer.h"
#include "Brothel.hpp"
#include "cMessageBox.h"
#include "cGold.h"
#include "DirPath.h"
#include "CLog.h"
#include "XmlMisc.h"
#include "cTariff.h"
#include "cCustomers.h"
#include "cGangs.h"
#include "Helper.hpp"
#include "InterfaceProcesses.h"
#include "strnatcmp.h"
#include "InterfaceGlobals.h"
#include "GirlManager.hpp"
#include "cInventory.h"

#include <sstream>

namespace WhoreMasterRenewal
{


Girl* girl_sort( Girl* girl, Girl** lastgirl ) // sort using merge sort (cause works well with linked lists)
{
    Girl* p, *q, *e, *tail;
    int insize, nmerges, psize, qsize, i;
    
    insize = 1;
    
    if( !girl )
    {
        *lastgirl = nullptr;
        return nullptr;
    }
    
    while( 1 )
    {
        p = girl;
        girl = nullptr;
        tail = nullptr;
        nmerges = 0;
        
        while( p )
        {
            nmerges++;
            q = p;
            psize = 0;
            
            for( i = 0; i < insize; i++ )
            {
                psize++;
                q = q->m_Next;
                
                if( !q )
                    break;
            }
            
            qsize = insize;
            
            while( psize > 0 || ( qsize > 0 && q ) )
            {
                if( psize == 0 )
                {
                    e = q;
                    q = q->m_Next;
                    qsize--;
                }
                else if( qsize == 0 || !q )
                {
                    e = p;
                    p = p->m_Next;
                    psize--;
                }
                else if( sGirlcmp( p, q ) <= 0 )
                {
                    e = p;
                    p = p->m_Next;
                    psize--;
                }
                else
                {
                    e = q;
                    q = q->m_Next;
                    qsize--;
                }
                
                if( tail )
                    tail->m_Next = e;
                else
                    girl = e;
                    
                e->m_Prev = tail;
                tail = e;
            }
            
            p = q;
        }
        
        tail->m_Next = nullptr;
        
        if( nmerges <= 1 )
        {
            // find the last girl in the list
            //*lastgirl = tail;
            Girl* tmp = girl;
            
            while( tmp->m_Next )
                tmp = tmp->m_Next;
                
            *lastgirl = tmp;
            return girl;
        }
        
        insize *= 2;
    }
}

int sGirlcmp( Girl* a, Girl* b )
{
    if( a == nullptr || b == nullptr )
        return 1;
        
    return strcmp( a->m_Realname.c_str(), b->m_Realname.c_str() );
}



BrothelManager::BrothelManager()
    : m_JobManager()
{
    for( int i = 0; i < MAXNUM_INVENTORY; i++ )
    {
        m_Inventory[i]  = nullptr;
        m_EquipedItems[i] = 0;
        m_NumItem[i]    = 0;
    }
    
    m_JobManager.Setup();
}

BrothelManager::~BrothelManager()
{
    Free();
}

void BrothelManager::Free()
{
    cConfig cfg;
    
    if( m_Prison )
        delete m_Prison;
        
    m_Prison            = nullptr;
    m_NumPrison         = 0;
    m_LastPrison        = nullptr;
    
    m_NumRunaways       = 0;
    
    if( m_Runaways )
        delete m_Runaways;
        
    m_Runaways          = nullptr;
    m_LastRunaway       = nullptr;
    
    m_NumInventory      = 0;
    
    for( int i = 0; i < MAXNUM_INVENTORY; i++ )
    {
        m_Inventory[i]  = nullptr;
        m_EquipedItems[i] = 0;
        m_NumItem[i]    = 0;
    }
    
    m_NumBrothels       = 0;
    m_AntiPregPotions   = 0;
    m_SupplyShedLevel   = 1;
    m_BribeRate         = 0;
    m_Influence         = 0;
    m_Bank              = 0;
    
    m_KeepPotionsStocked = false;
    m_HandmadeGoods     = 0;
    m_Beasts            = 0;
    m_AlchemyIngredients = 0;
    
    
    if( m_Objective )
        delete m_Objective;
        
    m_Objective         = nullptr;
    
    m_Dungeon.Free();
    
    m_Rivals.Free();
    
    if( m_Parent )
        delete m_Parent;
        
    m_Parent            = nullptr;
    m_Last              = nullptr;
}

void BrothelManager::check_raid()
{
    cRival* rival = nullptr;
    cRivalManager* rival_mgr = GetRivalManager();
    
    /*
     *  If the player's influence can shield him
     *  it only follows that the influence of his rivals
     *  can act to stitch him up
     *
     *  see if there exists a rival with infulence
     */
    if( rival_mgr->player_safe() == false )
    {
        rival = rival_mgr->get_influential_rival();
    }
    
    /*
     *  chance is based on how much suspicion is leveled at
     *  the player, less his influence at city hall.
     *
     *  And then modified back upwards by rival influence
     */
    int pc = m_Player.suspicion() - m_Influence;
    
    if( rival )
    {
        pc += rival->m_Influence / 4;
    }
    
    /*
     *  pc gives us the % chance of a raid
     *  let's do the "not raided" case first
     */
    if( g_Dice.percent( pc ) == false )
    {
        /*
         *      you are clearly a model citizen, sir
         *      and are free to go
         */
        return;
    }
    
    /*
     *  OK, the raid is on. Start formatting a message
     */
    std::stringstream ss;
    ss << "The local authorities perform a bust on your operations: ";
    
    /*
     *  if we make our influence check, the guard captain will be under
     *  orders from the mayor to let you off.
     *
     *  Let's make sure the player can tell
     */
    if( g_Dice.percent( m_Influence ) )
    {
        ss << "the guard captain lectures you on the importance of "
           << "crime prevention, whilst also passing on the Mayor's "
           << "heartfelt best wishes."
           ;
        m_Player.suspicion( -5 );
        g_MessageQue.AddToQue( ss.str(), 1 );
        return;
    }
    
    /*
     *  if we have a rival influencing things, it might not matter
     *  if the player is squeaky clean
     */
    if( m_Player.disposition() > 0 && g_Dice.percent( rival->m_Influence / 2 ) )
    {
        int fine = ( g_Dice % 1000 ) + 150;
        g_Gold.fines( fine );
        ss << "the guard captain condemns your operation as a "
           << "hotbed of criminal activity and fines you "
           << fine
           << " gold for 'living without due care and attention'."
           ;
        /*
         *      see if there's a girl using drugs he can nab
         */
        check_druggy_girl( ss );
        /*
         *      make sure the player knows why the captain is
         *      being so blatantly unfair
         */
        ss << "On his way out the captain smiles and says that the "
           << rival->m_Name
           << " send their regards."
           ;
        g_MessageQue.AddToQue( ss.str(), 1 );
        return;
    }
    
    /*
     *  if the player is basically a goody-goody type
     *  he's unlikely to have anything incriminating on
     *  the premises. 20 disposition should see him
     */
    if( g_Dice.percent( m_Player.disposition() * 5 ) )
    {
        ss << "they pronounce your operation to be "
           << "entirely in accordance with the law."
           ;
        m_Player.suspicion( -5 );
        g_MessageQue.AddToQue( ss.str(), 1 );
        return;
    }
    
    int nPlayer_Disposition = m_Player.disposition();
    
    if( nPlayer_Disposition > -10 )
    {
        int fine = ( g_Dice % 100 ) + 20;
        g_Gold.fines( fine );
        ss << "they find in technical violation of some health "
           << "and safety ordinances, and they fine you "
           << fine
           << " gold."
           ;
    }
    else if( nPlayer_Disposition > -30 )
    {
        int fine = ( g_Dice % 300 ) + 40;
        g_Gold.fines( fine );
        ss << "they find some minor criminalities and fine you "
           << fine
           << " gold.";
    }
    else if( nPlayer_Disposition > -50 )
    {
        int fine = ( g_Dice % 600 ) + 100;
        g_Gold.fines( fine );
        ss << "they find evidence of dodgy dealings and fine you "
           << fine
           << " gold."
           ;
    }
    else if( nPlayer_Disposition > -70 )
    {
        int fine = ( g_Dice % 1000 ) + 150;
        int bribe = ( g_Dice % 300 ) + 100;
        g_Gold.fines( fine + bribe );
        ss << "they find a lot of illegal activities and fine you "
           << fine
           << " gold, it also costs you an extra "
           << bribe
           << " to pay them off from arresting you."
           ;
    }
    else if( nPlayer_Disposition > -90 )
    {
        int fine = ( g_Dice % 1500 ) + 200;
        int bribe = ( g_Dice % 600 ) + 100;
        g_Gold.fines( fine + bribe );
        ss << "they find enough dirt to put you behind bars for life. "
           << "It costs you " << bribe << " to stay out of prison, "
           << " plus another "
           << fine
           << " in fines on top of that"
           ;
    }
    else
    {
        int fine = ( g_Dice % 2000 ) + 400;
        int bribe = ( g_Dice % 800 ) + 150;
        g_Gold.fines( fine + bribe );
        ss << "the captain declares your premises to be a sinkhole "
           << "of the utmost vice and depravity, and it is only "
           << "with difficulty that you dissuade him from seizing "
           << "all your property on the spot. You pay "
           << fine
           << " gold in fines, but only after slipping the captain "
           << bribe
           << " not to drag you off to prison."
           ;
    }
    
    /*
     *  check for a drug-using girl they can arrest
     */
    check_druggy_girl( ss );
    g_MessageQue.AddToQue( ss.str(), 1 );
}

int BrothelManager::TotalFame( Brothel* brothel )
{
    int total_fame = 0;
    Girl* current = brothel->m_Girls;
    
    while( current )
    {
        total_fame += g_Girls.GetStat( current, STAT_FAME );
        current = current->m_Next;
    }
    
    return total_fame;
}

bool BrothelManager::CheckScripts()
{
    Brothel* current = m_Parent;
    DirPath base = DirPath() << "Resources" << "Characters" << "";
    
    while( current )
    {
        Girl* girl;
        
        for( girl = current->m_Girls; girl; girl = girl->m_Next )
        {
            /*
             *          if no trigger for this girl, skip to the next one
             */
            if( !girl->m_Triggers.GetNextQueItem() )
            {
                continue;
            }
            
            std::string fileloc = base.c_str();
            fileloc += girl->m_Name;
            girl->m_Triggers.ProcessNextQueItem( fileloc );
            return true;
        }
        
        current = current->m_Next;
    }
    
    return false;
}

bool BrothelManager::UseAntiPreg( bool use )
{
    if( !use )
        return false;
        
    /*
     *  anti-preg potions, we probably should allow
     *  on-the-fly restocks. You can imagine someone
     *  noticing things are running low and
     *  sending a girl running to the shops to get
     *  a restock
     *
     *  that said, there's a good argument here for
     *  making this the matron's job, and giving it a
     *  chance dependent on skill level. Could have a
     *  comedy event where the matron forgets, or the
     *  girl forgets (or disobeys) and half a dozen
     *  girls get knocked up.
     *
     *  'course, we could do that anyway.. :)
     */
    if( m_KeepPotionsStocked )
    {
        g_Gold.consumable_cost( 10 );
        return true;
    }
    
    if( m_AntiPregPotions > 0 )
    {
        m_AntiPregPotions--;
        return true;
    }
    
    return false;
}

void BrothelManager::AddAntiPreg( int amount )
{
    m_AntiPregPotions += amount;
    
    if( m_AntiPregPotions > 700 )
        m_AntiPregPotions = 700;
}

void BrothelManager::AddGirl( int brothelID, Girl* girl )
{
    if( girl == nullptr )
        return;
        
    Brothel* current = m_Parent;
    
    while( current )
    {
        if( current->m_id == brothelID )
            break;
            
        current = current->m_Next;
    }
    
    g_Girls.RemoveGirl( girl, false );
    
    girl->m_Prev = girl->m_Next = nullptr;
    
    if( current->m_Girls )
    {
        girl->m_Prev = current->m_LastGirl;
        current->m_LastGirl->m_Next = girl;
        current->m_LastGirl = girl;
    }
    else
        current->m_LastGirl = current->m_Girls = girl;
        
    current->m_NumGirls++;
    //sort(current);
}

void BrothelManager::RemoveGirl( int brothelID, Girl* girl, bool deleteGirl )
{
    if( girl == nullptr )
        return;
        
    Brothel* current = m_Parent;
    
    while( current )
    {
        if( current->m_id == brothelID )
            break;
            
        current = current->m_Next;
    }
    
    Girl* t = current->m_Girls;
    bool found = false;
    
    while( t )
    {
        if( t == girl )
        {
            found = true;
            break;
        }
        
        t = t->m_Next;
    }
    
    t = nullptr; // MYR
    
    if( found == true )
    {
        if( girl->m_Next )
            girl->m_Next->m_Prev = girl->m_Prev;
            
        if( girl->m_Prev )
            girl->m_Prev->m_Next = girl->m_Next;
            
        if( girl == current->m_Girls )
            current->m_Girls = girl->m_Next;
            
        if( girl == current->m_LastGirl )
            current->m_LastGirl = girl->m_Prev;
            
        girl->m_Next = nullptr;
        girl->m_Prev = nullptr;
        
        if( deleteGirl )
        {
            delete girl;
            girl = nullptr;
        }
        
        current->m_NumGirls--;
    }
}

void BrothelManager::LoadDataLegacy( std::ifstream& ifs )
{
    Free();
    int temp;
    std::string message = "";
    
    // load the player
    //         ...................................................
    message = "***************** loading Player ******************";
    g_LogFile.write( message );
    m_Player.LoadPlayerLegacy( ifs );
    
    // load the dungeon
    //         ...................................................
    message = "***************** loading dungeon *****************";
    g_LogFile.write( message );
    m_Dungeon.LoadDungeonDataLegacy( ifs );
    
    // load preg potions, supply shed level, other goodies
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> m_AntiPregPotions >> m_SupplyShedLevel >> m_HandmadeGoods >> m_Beasts >> m_AlchemyIngredients;
    
    // load runaways
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    int numGirlsToLoad = 0;
    ifs >> numGirlsToLoad;
    m_NumPrison = 0;
    
    for( int i = 0; i < numGirlsToLoad; i++ )
    {
        Girl* rgirl = new Girl();
        g_Girls.LoadGirlLegacy( rgirl, ifs );
        
        AddGirlToRunaways( rgirl );
    }
    
    // load prison
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    numGirlsToLoad = 0;
    ifs >> numGirlsToLoad;
    m_NumPrison = 0;
    
    for( int i = 0; i < numGirlsToLoad; i++ )
    {
        Girl* pgirl = new Girl();
        g_Girls.LoadGirlLegacy( pgirl, ifs );
        
        AddGirlToPrison( pgirl );
    }
    
    // load bribe rate and bank
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> m_BribeRate >> m_Bank;
    
    // load objective
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> temp;
    
    if( temp == 1 )
    {
        if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
        
        m_Objective = new Objective();
        ifs >> m_Objective->m_Difficulty >> m_Objective->m_Limit >> m_Objective->m_Objective >> m_Objective->m_Reward >> m_Objective->m_SoFar >> m_Objective->m_Target;
    }
    
    // load rivals
    //         ...................................................
    message = "***************** Loading rivals *****************";
    g_LogFile.write( message );
    m_Rivals.LoadRivalsLegacy( ifs );
    
    // Load inventory
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> temp;
    m_NumInventory = temp;
    //         ...................................................
    message = "************ Loading players inventory ************";
    g_LogFile.write( message );
    sInventoryItem* tempitem = nullptr;
    int total_num = 0;
    
    for( int i = 0; i < m_NumInventory; i++ )
    {
        if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
        
        ifs.getline( buffer, sizeof( buffer ), '\n' );
        tempitem = g_InvManager.GetItem( buffer );
        
        message = "Loading item: ";
        message += buffer;
        g_LogFile.write( message );
        
        if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
        
        ifs >> temp;
        
        if( tempitem )
            m_EquipedItems[total_num] = temp;
            
        ifs >> temp;
        
        if( tempitem )
        {
            m_NumItem[total_num] = temp;
            
            m_Inventory[total_num] = tempitem;
            total_num++;
        }
    }
    
    m_NumInventory = total_num;
    
    // load potions restock
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> m_KeepPotionsStocked;
    
    // load alcohol restock
//  if (ifs.peek()=='\n') ifs.ignore(1,'\n');
//  ifs>>m_KeepAlcStocked;

    // Number of brothels
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    int numBrothelsToLoad = 0;
    ifs >> numBrothelsToLoad;
    m_NumBrothels = 0;
    //         ...................................................
    message = "***************** Loading brothels ****************";
    g_LogFile.write( message );
    
    for( int j = 0; j < numBrothelsToLoad; j++ )
    {
        Brothel* current = new Brothel();
        
        if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
        
        ifs.getline( buffer, sizeof( buffer ), '\n' );
        current->m_Name = buffer;
        
        message = "Loading brothel: ";
        message += current->m_Name;
        g_LogFile.write( message );
        
        // load variables for sex restrictions
        if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
        
        ifs >> temp;
        
        if( temp == 1 )
            current->m_RestrictAnal = true;
        else
            current->m_RestrictAnal = false;
            
        ifs >> temp;
        
        if( temp == 1 )
            current->m_RestrictBDSM = true;
        else
            current->m_RestrictBDSM = false;
            
        ifs >> temp;
        
        if( temp == 1 )
            current->m_RestrictBeast = true;
        else
            current->m_RestrictBeast = false;
            
        ifs >> temp;
        
        if( temp == 1 )
            current->m_RestrictGroup = true;
        else
            current->m_RestrictGroup = false;
            
        ifs >> temp;
        
        if( temp == 1 )
            current->m_RestrictNormal = true;
        else
            current->m_RestrictNormal = false;
            
        ifs >> temp;
        
        if( temp == 1 )
            current->m_RestrictLesbian = true;
        else
            current->m_RestrictLesbian = false;
            
        if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
        
        ifs >> temp;
        current->m_AdvertisingBudget = temp;
        ifs >> temp;
        current->m_Bar = temp;
        ifs >> temp;
        current->m_Fame = temp;
        ifs >> temp;
        current->m_GamblingHall = temp;
        ifs >> temp;
        current->m_Happiness = temp;
        ifs >> temp;
        current->m_HasBarStaff = temp;
        ifs >> current->m_id;
        ifs >> temp;
        current->m_HasGambStaff = temp;
        ifs >> temp; // m_MovieRunTime - we don't support movies
        ifs >> temp;
        current->m_NumGirls = temp;
        ifs >> temp;
        current->m_NumRooms = temp;
        ifs >> temp; // m_ShowQuality - we don't support movies
        ifs >> temp; // m_ShowTime - we don't support movies
//      ifs>>current->m_Upkeep;
        ifs >> current->m_Filthiness;
        ifs >> current->m_SecurityLevel;
        
        //ifs>>current->m_Finance;
        current->m_Finance.loadGoldLegacy( ifs );
        
        // load building qualities
        if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
        
        for( u_int i = 0; i < NUMJOBTYPES; i++ )
            ifs >> current->m_BuildingQuality[i];
            
        if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
        
        int count = 0;
        ifs >> count;
        
        /* Movies - let's just ignore this for now... */
        for( int i = 0; i < count; i++ )
        {
            if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
            
            ifs >> temp;
            //when you create a new movie, you set m_Quality to quality*0.5
            //but you directly save m_Quality, so this undoes the division
            temp *= 2;
        }
        
        // Load girls
        if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
        
        numGirlsToLoad = current->m_NumGirls;
        current->m_NumGirls = 0;
        
        for( int i = 0; i < numGirlsToLoad; i++ )
        {
            Girl* girl = new Girl();
            g_Girls.LoadGirlLegacy( girl, ifs );
            
            message = "Loading girl: ";
            message += girl->m_Realname;
            g_LogFile.write( message );
            
            current->AddGirl( girl );
        }
        
        //current->building.load(ifs);
        
        AddBrothel( current );
    } // load a brothel
}

bool BrothelManager::LoadDataXML( TiXmlHandle hBrothelManager )
{
    Free();//everything should be init even if we failed to load an XML element
    //watch out, this frees dungeon and rivals too
    
    TiXmlElement* pBrothelManager = hBrothelManager.ToElement();
    
    if( pBrothelManager == nullptr )
    {
        return false;
    }
    
    std::string message = "";
    
    // load the player
    //         ...................................................
    message = "***************** loading Player ******************";
    g_LogFile.write( message );
    m_Player.LoadPlayerXML( hBrothelManager.FirstChild( "Player" ) );
    
    // load the dungeon
    //         ...................................................
    message = "***************** loading dungeon *****************";
    g_LogFile.write( message );
    m_Dungeon.LoadDungeonDataXML( hBrothelManager.FirstChild( "Dungeon" ) );
    
    // load preg potions, supply shed level, other goodies
    pBrothelManager->QueryIntAttribute( "AntiPregPotions", &m_AntiPregPotions );
    pBrothelManager->QueryIntAttribute( "SupplyShedLevel", &m_SupplyShedLevel );
    pBrothelManager->QueryIntAttribute( "HandmadeGoods", &m_HandmadeGoods );
    pBrothelManager->QueryIntAttribute( "Beasts", &m_Beasts );
    pBrothelManager->QueryIntAttribute( "AlchemyIngredients", &m_AlchemyIngredients );
    
    // load runaways
    m_NumRunaways = 0;
    TiXmlElement* pRunaways = pBrothelManager->FirstChildElement( "Runaways" );
    
    if( pRunaways )
    {
        for( TiXmlElement* pGirl = pRunaways->FirstChildElement( "Girl" );
                pGirl != nullptr;
                pGirl = pGirl->NextSiblingElement( "Girl" ) ) // load each girl and add her
        {
            Girl* rgirl = new Girl();
            bool success = rgirl->LoadGirlXML( TiXmlHandle( pGirl ) );
            
            if( success == true )
            {
                AddGirlToRunaways( rgirl );
            }
            else
            {
                delete rgirl;
                continue;
            }
        }
    }
    
    // load prison
    m_NumPrison = 0;
    TiXmlElement* pPrisonGirls = pBrothelManager->FirstChildElement( "PrisonGirls" );
    
    if( pPrisonGirls )
    {
        for( TiXmlElement* pGirl = pPrisonGirls->FirstChildElement( "Girl" );
                pGirl != nullptr;
                pGirl = pGirl->NextSiblingElement( "Girl" ) ) // load each girl and add her
        {
            Girl* pgirl = new Girl();
            bool success = pgirl->LoadGirlXML( TiXmlHandle( pGirl ) );
            
            if( success == true )
            {
                AddGirlToPrison( pgirl );
            }
            else
            {
                delete pgirl;
                continue;
            }
        }
    }
    
    // load bribe rate and bank
    pBrothelManager->QueryValueAttribute<long>( "BribeRate", &m_BribeRate );
    pBrothelManager->QueryValueAttribute<long>( "Bank", &m_Bank );
    
    // load objective
    TiXmlElement* pObjective = pBrothelManager->FirstChildElement( "Objective" );
    
    if( pObjective )
    {
        m_Objective = new Objective();
        pObjective->QueryIntAttribute( "Difficulty", &m_Objective->m_Difficulty );
        pObjective->QueryIntAttribute( "Limit", &m_Objective->m_Limit );
        pObjective->QueryValueAttribute<u_int>( "Objective", &m_Objective->m_Objective );
        pObjective->QueryIntAttribute( "Reward", &m_Objective->m_Reward );
        pObjective->QueryIntAttribute( "SoFar", &m_Objective->m_SoFar );
        pObjective->QueryIntAttribute( "Target", &m_Objective->m_Target );
    }
    
    // load rivals
    //         ...................................................
    message = "***************** Loading rivals *****************";
    g_LogFile.write( message );
    m_Rivals.LoadRivalsXML( hBrothelManager.FirstChild( "Rival_Manager" ) );
    
    // Load inventory
    //         ...................................................
    message = "************ Loading players inventory ************";
    g_LogFile.write( message );
    //now would be a great time to move this to cPlayer
    LoadInventoryXML( hBrothelManager.FirstChild( "Inventory" ),
                      m_Inventory, m_NumInventory, m_EquipedItems, m_NumItem );
                      
    // load potions restock
    pBrothelManager->QueryValueAttribute<bool>( "KeepPotionsStocked", &m_KeepPotionsStocked );
    
    // load alcohol restock
//  if (ifs.peek()=='\n') ifs.ignore(1,'\n');
//  ifs>>m_KeepAlcStocked;

    //         ...................................................
    message = "***************** Loading brothels ****************";
    g_LogFile.write( message );
    m_NumBrothels = 0;
    TiXmlElement* pBrothels = pBrothelManager->FirstChildElement( "Brothels" );
    
    if( pBrothels )
    {
        for( TiXmlElement* pBrothel = pBrothels->FirstChildElement( "Brothel" );
                pBrothel != nullptr;
                pBrothel = pBrothel->NextSiblingElement( "Brothel" ) )
        {
            Brothel* current = new Brothel();
            bool success = current->LoadBrothelXML( TiXmlHandle( pBrothel ) );
            
            if( success == true )
            {
                AddBrothel( current );
            }
            else
            {
                delete current;
                continue;
            }
            
        } // load a brothel
    }
    
    return true;
}

TiXmlElement* BrothelManager::SaveDataXML( TiXmlElement* pRoot )
{
    TiXmlElement* pBrothelManager = new TiXmlElement( "Brothel_Manager" );
    pRoot->LinkEndChild( pBrothelManager );
    std::string message;
    
    // save the Player
    //         ...................................................
    message = "************* saving Player data ******************";
    g_LogFile.write( message );
    m_Player.SavePlayerXML( pBrothelManager );
    
    // save the dungeon
    //         ...................................................
    message = "************* saving dungeon data *****************";
    g_LogFile.write( message );
    m_Dungeon.SaveDungeonDataXML( pBrothelManager );
    
    // save preg potions, supply shed level, other goodies
    pBrothelManager->SetAttribute( "AntiPregPotions", m_AntiPregPotions );
    pBrothelManager->SetAttribute( "SupplyShedLevel", m_SupplyShedLevel );
    pBrothelManager->SetAttribute( "HandmadeGoods", m_HandmadeGoods );
    pBrothelManager->SetAttribute( "Beasts", m_Beasts );
    pBrothelManager->SetAttribute( "AlchemyIngredients", m_AlchemyIngredients );
    
    // save runaways
    TiXmlElement* pRunaways = new TiXmlElement( "Runaways" );
    pBrothelManager->LinkEndChild( pRunaways );
    Girl* rgirl = m_Runaways;
    
    while( rgirl )
    {
        rgirl->SaveGirlXML( pRunaways );
        rgirl = rgirl->m_Next;
    }
    
    // save prison
    TiXmlElement* pPrison = new TiXmlElement( "PrisonGirls" );
    pBrothelManager->LinkEndChild( pPrison );
    Girl* pgirl = m_Prison;
    
    while( pgirl )
    {
        pgirl->SaveGirlXML( pPrison );
        pgirl = pgirl->m_Next;
    }
    
    // save bribe rate and bank
    pBrothelManager->SetAttribute( "BribeRate", m_BribeRate );
    pBrothelManager->SetAttribute( "Bank", m_Bank );
    
    // save objective
    if( m_Objective )
    {
        TiXmlElement* pObjective = new TiXmlElement( "Objective" );
        pBrothelManager->LinkEndChild( pObjective );
        pObjective->SetAttribute( "Difficulty", m_Objective->m_Difficulty );
        pObjective->SetAttribute( "Limit", m_Objective->m_Limit );
        pObjective->SetAttribute( "Objective", m_Objective->m_Objective );
        pObjective->SetAttribute( "Reward", m_Objective->m_Reward );
        pObjective->SetAttribute( "SoFar", m_Objective->m_SoFar );
        pObjective->SetAttribute( "Target", m_Objective->m_Target );
    }
    
    // save rivals
    //         ...................................................
    message = "***************** Saving rivals *******************";
    g_LogFile.write( message );
    m_Rivals.SaveRivalsXML( pBrothelManager );
    
    // save inventory
    //         ...................................................
    message = "************** Saving players inventory ***********";
    g_LogFile.write( message );
    TiXmlElement* pInventory = new TiXmlElement( "Inventory" );
    pBrothelManager->LinkEndChild( pInventory );
    SaveInventoryXML( pInventory, m_Inventory, MAXNUM_INVENTORY, m_EquipedItems, m_NumItem );
    
    // save potions restock
    pBrothelManager->SetAttribute( "KeepPotionsStocked", m_KeepPotionsStocked );
    
    // save alcohol restock
//  ofs<<m_KeepAlcStocked<<std::endl;

    // save brothels
    TiXmlElement* pBrothels = new TiXmlElement( "Brothels" );
    pBrothelManager->LinkEndChild( pBrothels );
    Brothel* current = m_Parent;
    //         ...................................................
    message = "***************** Saving brothels *****************";
    g_LogFile.write( message );
    
    while( current )
    {
        message = "Saving brothel: ";
        message += current->m_Name;
        g_LogFile.write( message );
        
        current->SaveBrothelXML( pBrothels );
        current = current->m_Next;
    }
    
    return pBrothelManager;
}

void BrothelManager::NewBrothel( int NumRooms )
{
    Brothel* newBroth = new Brothel();
    newBroth->m_NumRooms = NumRooms;
    newBroth->m_Next = nullptr;
    
    AddBrothel( newBroth );
}

void BrothelManager::AddBrothel( Brothel* newBroth )
{
    if( m_Parent )
    {
        m_Last->m_Next = newBroth;
        newBroth->m_id = m_Last->m_id + 1;
        m_Last = newBroth;
    }
    else
    {
        m_Parent = m_Last =  newBroth;
        newBroth->m_id = 0;
    }
    
    m_NumBrothels++;
}

void BrothelManager::DestroyBrothel( int ID )
{
    Brothel* current = m_Parent;
    
    if( current->m_id == ID )
    {
        m_Parent = current->m_Next;
        current->m_Next = nullptr;
        delete current;
        return;
    }
    
    while( current->m_Next )
    {
        if( current->m_Next->m_id == ID )
            break;
            
        current = current->m_Next;
    }
    
    if( current )
    {
        Brothel* temp = current->m_Next;
        
        current->m_Next = temp->m_Next;
        
        temp->m_Next = nullptr;
        delete temp;
        
        m_NumBrothels--;
    }
}

void BrothelManager::check_rivals()
{
    int num_rivals = m_Rivals.GetNumRivals();
    static bool peace = false;
    
    /*
     *  a full set of rivals = nothing to do
     */
    if( num_rivals > 5 )
    {
        return;
    }
    
    /*
     *  if there are no rivals, and we were not
     *  at peace last turn, peace has broken out
     */
    if( num_rivals == 0 && !peace )
    {
        peace = true;
        peace_breaks_out();
    }
    
    /*
     *  we only create new rivals after the game has
     *  been won
     */
    if( m_Player.m_WinGame == false )
    {
        return;
    }
    
    /*
     *  create new random rival
     */
    if( g_Dice.percent( 70 ) )
    {
        return;     // or not!
    }
    
    /*
     *  flag the war as on again, (should be a field somewhere)
     *  create a new rival and tell the player the good news
     */
    peace = false;
    m_Rivals.CreateRandomRival();
    g_MessageQue.AddToQue( new_rival_text(), 2 );
}

std::string BrothelManager::new_rival_text()
{
    std::stringstream ss;
    
    enum
    {
        Slaver      = 0,
        Gladiator   = 1,
        Goon        = 2,
        Slave       = 3,
        Mage        = 4,
        Demon       = 5,
        Priest      = 6,
        Noble       = 7,
        Technologist    = 8,
        Patriarch   = 9,    // or Matriarch
        MaxChallengers  = 10
    };
    
    bool male = g_Dice.percent( 75 );
    /*
     *  let's put the gender specific terms in
     *  variables. Might make the code cleaner
     */
    std::string man, boy, He, he, him, his, sorcerer, gladiator;
    std::string fellow, patriarch;
    
    if( male )
    {
        He  = "He";
        he  = "he";
        him = "him";
        his = "his";
        man = "man";
        boy = "boy";
        sorcerer = "sorcerer";
        gladiator = "gladiator";
        fellow = "fellow";
        patriarch = "patriarch ";
    }
    else
    {
        He  = "She";
        he  = "she";
        him = "her";
        his = "her";
        man = "woman";
        boy = "girl";
        sorcerer = "sorceress";
        gladiator = "gladiatrix";
        /*
         *      not sure what the feminine of "fellow" is
         *      I did wonder about "fellatrix"...
         */
        fellow = "wench";
        patriarch = "matriarch ";
    }
    
    switch( g_Dice.random( MaxChallengers ) )
    {
    case Slaver:
        ss << "A lieutenant reports that one of the "
           << "professional slavers, finding customers "
           << "be scarce, has taken to whoring out "
           << his << " slavegirls to make ends meet."
           << "Your men arranged a meet with " << him
           << " in order to explain your position "
           << "on the subject, but the discussion "
           << "did not go well, ending with bared "
           << "steel and threats of blood."
           << "\n\n"
           << "It would seem you have a challenger."
           ;
        break;
        
    case Gladiator:
        ss << "Ask any Crossgate sports fan who rules the Arenas ";
        ss << "of the city.  Almost always, the answer will be ";
        ss << "the same. For five long years one " << gladiator << " has ";
        ss << "stood " << his << " ground on the bloody sands and defied ";
        ss << "all who came before " << him << ".";
        ss << "\n\n";
        ss << "Last week, the " << gladiator << " bought " << his ;
        ss << " freedom from ";
        ss << "the arena, and chose to celebrate the occasion at ";
        ss << "one of your brothels.  Sadly, an overindulgence ";
        ss << "in wine led to harsh words and a rash vow to ";
        ss << "show you how a whorehouse SHOULD be run. ";
        ss << "\n\n";
        ss << "With anyone else, the matter would have ended ";
        ss << "when the morning brought sobriety. But this is ";
        ss << "a " << man << " who has never turned " << his << " back ";
        ss << "on any sort of challenge. With wealthy admirers ";
        ss << "supplying premises and finance, and with a handful ";
        ss << "of arena veterans to provide the core of " << his;
        ss << " enforcers, this ";
        ss << "is a challenger you would be foolish to ignore.";
        break;
        
    case Goon:
        ss <<   "The " << boy << " was just skin and bones; a dull eyed "
           "waif from gutters of Sleaze Street, a dozen like "
           << him << " on any street corner. But put a knife in "
           << his << " hands and the " << boy << " became an artist, "
           "painting effortless masterpieces in blood and "
           "greased lightning. "
           "\n\n"
           "Quickly recruited into one of the goon squads, "
           "it soon became apparent that behind that flat "
           "unblinking stare, there lurked a mind almost "
           "as keen as " << his << " blades. The " << boy << " rose quickly, "
           "coming to head " << his << " own squad before becoming "
           "one of your trusted lieutenants. If only " << his << " "
           "ambition had stopped there... "
           "\n\n"
           "" << ( male ? "His" : "Her" ) << " challenge "
           "was almost over before it began; "
           "for you that is. That you still live says more "
           "about the skill of your healers than any talent "
           "you might lay claim to.  Your newest rival is not "
           "only a deadly fighter and a clever strategist, "
           "but one who knows your operation, inside and out. "
           "\n\n"
           "This will not be easy."
           ;
        break;
        
    case Slave:
        ss <<   "There are ways to beat a slaver tattoo. It wouldn't "
           "do were that to become widely known, of course. "
           "Nevertheless there are ways around it. "
           "One such is to find an area of unstable spacetime. "
           "Do it right, and you can overload the tracking "
           "spell, and the enchantment just falls apart"
           "This is, of course wildly dangerous, but many "
           "escapees nevertheless head straight for the "
           "Crossgate sewers, which on a bad day can give "
           "the catacombs a run for their money."
           "\n\n"
           "Over time, a community of ecapees has grown up "
           "in the sewers, survivor types, grown hardy in "
           "the most hostile environment. And as long as they "
           "stay down there, no one much minds. If nothing else "
           "they keep the monster population down. But now "
           "they seem to be organising a crusade. Against "
           "slavery. Against exploitation. Against you."
           "\n\n"
           "Rumour has it that their leader is one of your "
           "offspring, conceived of rape, born into slavery. "
           "True or not, this new factions seems determined "
           "to bring about your downfall."
           "\n\n"
           "This time, as the bards would say, it is personal."
           ;
        break;
        
    case Mage:
        ss <<   "The " << sorcerer << " blew into town with a travelling ";
        ss <<   "entertainer show, promising exotic pleasures ";
        ss <<   "and the taste of forbidden fruit. But behind the ";
        ss <<   "showman's patter and the coloured smoke, the pleasures ";
        ss <<   "on offer were of a distinctly carnal nature, and no ";
        ss <<   "more exotic than those you yourself could offer.";
        ss <<   "\n\n";
        ss <<   "For a travelling show, this need not be a ";
        ss <<   "problem. For a week, or even two, you can ";
        ss <<   "stand to see a little competition. However, ";
        ss <<   "the newcomer has been here a month now and ";
        ss <<   "shows no sign of moving on. On the contrary, ";
        ss <<   "he appears to be shopping for permanent premises.";
        ss <<   "\n\n";
        ss <<   "With this in mind, you send some men ";
        ss <<   "to explain the situation. ";
        ss <<   "To everyone's surprise, it turns out ";
        ss <<   "that behind the glib charlatanry, there lies ";
        ss <<   "genuine magecraft, most likely tantric in nature.";
        ss <<   "\n\n";
        ss <<   "In your organisation you have no shortage of ";
        ss <<   "mages. Any fighting force in Crossgate needs ";
        ss <<   "a battle mage or two. This newcomer however ";
        ss <<   "operates on a level far beyond what you are ";
        ss <<   "used to. And he seems determined to stay, and ";
        ss <<   "challenge you for control of the city.";
        
        
        break;
        
    case Priest:
        break;
        
    case Noble:
        ss << "They say " << he << " is a noble, an exile from " << his << " ";
        ss << "native land. Certainly, " << he << " has the manners of a courtier ";
        ss << "and the amused weariness of the jaded dilettante.";
        ss << "\n\n";
        ss << "And yet it seems there is steel behind the foppery, as ";
        ss << "many a Crossgate duelist has learned. And a wit to ";
        ss << "match the blade as well. An admirable " << fellow << " this, ";
        ss << "one you would be pleased to call 'friend', if only ...";
        ss << "\n\n";
        ss << "Earlier this week, your men were explaining to a handful of ";
        ss << "freelance scrubbers how ";
        ss << "prostitution worked in this city. If only " << he << " had not chosen ";
        ss << "to take the women's side against your men. If only ";
        ss << his << " rash defiance had not caught the imagination of ";
        ss << "the city's duellists.";
        ss << "\n\n";
        ss << "Alas, such was not to be.";
        ss << "\n\n";
        ss << "En Garde!";
        break;
        
    case Technologist:
        ss << "From the distant city of Abby's Crossing comes a new ";
        ss << "rival to challenge for your throne, wielding some ";
        ss << "strange non-magic " << he << " calls 'technology', ";
        ss << "an alien art of smoke and steam and noise and ";
        ss << "lighting; one they say functions strangely in Mundiga, ";
        ss << "when it chooses to work at all.";
        ss << "\n\n";
        ss << "But the hollow metal men that make up " << his;
        ss << " enforcers would seem to work with deadly efficicency ";
        ss << "and the strange collapsible maze " << he << " calls a ";
        ss << "'tesseract' seems to share many properties with the ";
        ss << "catacombs under your headquarters. Then there are ";
        ss << "rumours of strange procedures that can break a ";
        ss << "slavegirl's will, far faster than the most skilled of ";
        ss << "Crossgate's torturers.";
        ss << "\n\n";
        ss << "In short, far from unreliable, " << his << " arts seem deadly ";
        ss << "efficient to you. You have no idea what other surprises ";
        ss << "this otherworldly artisan may have up " << his << " sleeve, but ";
        ss << "one thing is for certain: this challenge may not go ";
        ss << "unanswered.";
        break;
        
    case Patriarch:
        ss << "Outside the walls of Crossgate, there is a shanty-town ";
        ss << "maze of tumbledown hovels, teeming with the poorest ";
        ss << "and most desperate of the City's inhabitants. Polygamy ";
        ss << "and incest are rife here, and extended families can ";
        ss << "run into the hundreds";
        ss << "\n\n";
        ss << "One such family is ruled by the iron will of a ";
        ss << "dreadful old " << patriarch << " with a well earned ";
        ss << "reputation for utter ruthlessness. For years " << he << " ";
        ss << "has sent " << his << " progeny to the city markets, to trade, to ";
        ss << "steal, to bring back money for the clan in any way ";
        ss << "they can.";
        ss << "\n\n";
        ss << "Now it seems they are expanding their operation to ";
        ss << "include organised prostitution. Bad move.";
        ss << "\n\n";
        ss << "Something about the " << patriarch << "'s operation ";
        ss << "disturbs you. There is a coldness in the way " << he << " ";
        ss << "sends sons and grandsons out to die for " << him << "; the way ";
        ss << he << "casually rapes and enslaves " << his << " own daughters and ";
        ss << "granddaughters before sending them off to whore for ";
        ss << him << ". This " << man << " holds up a mirror to what you are ";
        ss << "- or perhaps to what you could easily become. The ";
        ss << "image it presents is far from flattering.";
        ss << "\n\n";
        ss << "Personal feelings aside, this is a situation that ";
        ss << "can only get worse. The time to end this, is now.";
        break;
        
    case Demon:
        ss << "Somewhere in Crossgate, a hand trembled ";
        ss << "inscribing a pentagram; a tongue stumbled over ";
        ss << "the nine syllables of the charm of binding. ";
        ss << "A magical being slipped his arcane bonds ";
        ss << "and slaughtered those mages foolish enough to dream ";
        ss << "they might command it.";
        ss << "\n\n";
        ss << "A demon lord now stalks the streets of the city.";
        ss << "\n\n";
        ss << "Which, in itself, is not so big a deal. It is not of ";
        ss << "unheard that the aristocracy of Hell should find ";
        ss << "themselves stumbling dazed and confused through ";
        ss << "Crossgate market.  They just tend to recover quickly ";
        ss << "and promptly open a portal home.";
        ss << "\n\n";
        ss << "But not this one. This one chooses to briefly linger, ";
        ss << "to partake of Crossgate society and seek such ";
        ss << "amusements as the city can offer. Unfortunately, it ";
        ss << "seems the demon finds amusement trafficking in human ";
        ss << "misery and human sex. As do you, in the eyes of many.";
        ss << "\n\n";
        ss << "For a demon, 'briefly' may be anything from a ";
        ss << "day to a thousand years. You cannot afford to wait ";
        ss << "until it grows bored. A demon lord is a formidable ";
        ss << "opponent, but to ignore this challenge will send ";
        ss << "entirely the wrong signal to the other would be ";
        ss << "whore-masters in the city.";
        ss << "\n\n";
        ss << "Like it or not, this means war.";
        break;
        
    default:
        g_LogFile.ss() << "switch (g_Dice.random(MaxChallengers)): unknown value\n" << __FILE__ << " " << __LINE__ << "\n";
    }
    
    return ss.str();
}

void BrothelManager::peace_breaks_out()
{
    /*
     *  if the PC already won, this is just an minor outbreak
     *  of peace in the day-to-day feuding in crossgate
     */
    if( m_Player.m_WinGame )
    {
        g_MessageQue.AddToQue(
            "The last of your challengers has been "
            "overthrown. Your domination of Crossgate "
            "is absolute.\n\n"
            "Until the next time, that is...", 1
        );
        return;
    }
    
    /*
     *  otherwise, the player has just won
     *  flag it as such
     */
    m_Player.m_WinGame = true;
    /*
     *  let's have a bit of chat to mark the event
     */
    std::string s = ""
                    "The last of your father's killers has been "
                    "brought before you for judgement. None "
                    "remain who would dare to oppose you. For all intents "
                    "and purposes, the city is yours."
                    "\n\n"
                    "Whether or not your father will rest easier for "
                    "your efforts, you cannot say, but now, with the "
                    "city at your feet, you feel sure he would be proud "
                    "of you at this moment."
                    "\n\n"
                    "But pride comes before a fall, and in Crossgate, "
                    "complacency kills. The city's slums and slave markets "
                    "and the fighting pits are full of hungry young bloods "
                    "burning to make their mark on the world, and any one of "
                    "them could rise to challenge you at any time."
                    "\n\n"
                    "You may have seized the city, but holding on to it "
                    "is never going to be easy."
                    ;
    g_MessageQue.AddToQue( s, 1 );
    return;
}

void BrothelManager::UpdateBrothels()
{
    cTariff tariff;
    Brothel* current = m_Parent;
    
    m_TortureDoneFlag = false;                          //WD: Reset flag each day is set in WorkTorture()
    
    UpdateBribeInfluence();
    
    while( current )
    {
        // reset the data
        current->m_Happiness = current->m_MiscCustomers = current->m_TotalCustomers = 0;
        current->m_Finance.zero();
        current->m_Events.Clear();
        
        // Clear the girls' events from the last turn
        Girl* cgirl = current->m_Girls;
        
        while( cgirl )
        {
            cgirl->m_Events.Clear();
            cgirl = cgirl->m_Next;
        }
        
        // handle advertising jobs to determine advertising multiplier
        m_JobManager.do_advertising( current );
        
        // Generate customers for the brothel for the day shift and update girls
        g_Customers.GenerateCustomers( current, 0 );
        current->m_TotalCustomers += g_Customers.GetNumCustomers();
        UpdateGirls( current, 0 );
        
        // update the girls and satisfy the customers for this brothel during the night
        g_Customers.GenerateCustomers( current, 1 );
        current->m_TotalCustomers += g_Customers.GetNumCustomers();
        UpdateGirls( current, 1 );
        
        // get the misc customers
        current->m_TotalCustomers += current->m_MiscCustomers;
        
        std::string data = "";
        data += toString( ( current->m_TotalCustomers + current->m_MiscCustomers ) );
        data += " customers visited the building.";
        
        current->m_Events.AddMessage( data, IMGTYPE_PROFILE, EVENT_BROTHEL );
        
        // empty rooms cost 2 gold to maintain
        current->m_Finance.building_upkeep( tariff.empty_room_cost( current ) );
        
        // update brothel stats
        if( current->m_NumGirls > 0 )
            current->m_Fame = ( TotalFame( current ) / current->m_NumGirls );
            
        if( current->m_Happiness > 0 && g_Customers.GetNumCustomers() )
            current->m_Happiness = current->m_Happiness / current->m_TotalCustomers;
            
        if( current->m_Happiness > 100 )
            current->m_Happiness = 100;
            
        // advertising costs are set independently for each brothel
        current->m_Finance.advertising_costs( tariff.advertising_costs( current->m_AdvertisingBudget ) );
        
        std::string data2 = "";
        data2 += "Your advertising budget for this brothel is ";
        data2 += toString( current->m_AdvertisingBudget );
        data2 += " gold.";
        
        if( tariff.advertising_costs( current->m_AdvertisingBudget ) != current->m_AdvertisingBudget )
        {
            data2 += " However, due to your configuration, you instead had to pay ";
            data2 += toString( tariff.advertising_costs( current->m_AdvertisingBudget ) );
            data2 += " gold.";
        }
        
        current->m_Events.AddMessage( data2, IMGTYPE_PROFILE, EVENT_BROTHEL );
        
        // update the global cash
        g_Gold.brothel_accounts( current->m_Finance, current->m_id );
        
        /*if(current->m_Filthiness > 100)   // MYR: Lets make this a little harsher
            current->m_Filthiness=100;
        
        else*/ if( current->m_Filthiness < 0 )
            current->m_Filthiness = 0;
            
        // goto the next brothel
        current = current->m_Next;
    }
    
    // Update the bribe rate
    g_Gold.bribes( m_BribeRate );
    
    if( m_Runaways )
    {
        Girl* rgirl = m_Runaways;
        
        while( rgirl )
        {
            if( rgirl->m_RunAway > 0 )
            {
                // there is a chance the authorities will catch her if she is branded a slave
                if( rgirl->m_States & ( 1 << STATUS_SLAVE ) )
                {
                    if( ( g_Dice % 100 ) + 1 < 5 )
                    {
                        // girl is recaptured and returned to you
                        Girl* temp = rgirl;
                        rgirl = temp->m_Next;
                        RemoveGirlFromRunaways( temp );
                        m_Dungeon.AddGirl( temp, DUNGEON_GIRLRUNAWAY );
                        g_MessageQue.AddToQue( "A runnaway slave has been recaptured by the authorities and returned to you.", 1 );
                        continue;
                    }
                }
                
                rgirl->m_RunAway--;
            }
            else    // add her back to girls
            {
                Girl* temp = rgirl;
                rgirl = temp->m_Next;
                temp->m_NightJob = temp->m_DayJob = JOB_RESTING;
                RemoveGirlFromRunaways( temp );
                g_Girls.AddGirl( temp );
                continue;
            }
            
            rgirl = rgirl->m_Next;
        }
    }
    
    if( m_Prison )
    {
        if( ( g_Dice % 100 ) + 1 < 10 ) // 10% chance of someone being released
        {
            Girl* girl = m_Prison;
            RemoveGirlFromPrison( girl );
            g_Girls.AddGirl( girl );
        }
    }
    
    // keep gravitating player suspicion to 0
    if( m_Player.suspicion() > 0 )
        m_Player.suspicion( -1 );
    else if( m_Player.suspicion() < 0 )
        m_Player.suspicion( 1 );
        
    // is the player under suspision by the authorities
    if( m_Player.suspicion() > 20 )
    {
        check_raid();
    }
    
    // incraese the bank gold by 02%
    if( m_Bank > 0 )
    {
        int amount = static_cast<int>( m_Bank * 0.002f );
        m_Bank += amount;
        /*
         *      bank iterest isn't added to the gold value
         *      but it can be recorded for reporting purposes
         */
        g_Gold.bank_interest( amount );
    }
    
    // get money from currently extorted businesses
    if( g_Gangs.GetNumBusinessExtorted() > 0 )
    {
        long gold = g_Gangs.GetNumBusinessExtorted() * INCOME_BUSINESS;
        int num = g_Dice % 15;
        
        if( num == 1 )
            gold -= INCOME_BUSINESS;
            
        std::string message = "You gain ";
        message += toString( gold );
        message += " gold from the ";
        message += toString( g_Gangs.GetNumBusinessExtorted() );
        message += " businesses under your control.\n";
        g_Gold.extortion( gold );
        
        if( num == 1 )
        {
            message += "A man cannot pay so he sells you his daughter.";
            m_Dungeon.AddGirl( g_Girls.CreateRandomGirl( 17, false ), DUNGEON_GIRLKIDNAPPED );
        }
        
        g_MessageQue.AddToQue( message, 3 );
    }
    
    do_tax();
    
    // update the people in the dungeon
    m_Dungeon.Update();
    
    check_rivals();
    
    long totalProfit = g_Gold.total_profit();
    
    if( totalProfit == 0 )
    {
        g_MessageQue.AddToQue( "You are breaking even (made as much money as you spent)", 3 );
    }
    else
    {
        std::stringstream ss;
        
        ss << "you made a overall profit of "
           << totalProfit
           << " gold.";
        g_MessageQue.AddToQue( ss.str(), 2 );
    }
    
    // MYR: I'm really curious about what goes in these if statements
    
    if( g_Gangs.GetNumBusinessExtorted() < 40 && GetNumBrothels() >= 2 )
    {
    }
    
    if( g_Gangs.GetNumBusinessExtorted() < 70 && GetNumBrothels() >= 3 )
    {
    }
    
    if( g_Gangs.GetNumBusinessExtorted() < 100 && GetNumBrothels() >= 4 )
    {
    }
    
    if( g_Gangs.GetNumBusinessExtorted() < 140 && GetNumBrothels() >= 5 )
    {
    }
    
    if( g_Gangs.GetNumBusinessExtorted() < 170 && GetNumBrothels() >= 6 )
    {
    }
    
    // update objectives or maybe create a new one
    if( GetObjective() )
        UpdateObjective();
    else
    {
        if( ( g_Dice % 100 ) + 1 < 45 )
            CreateNewObjective();
    }
}

void BrothelManager::UpdateGirls( Brothel* brothel, int DayNight )
{
    Girl* current = brothel->m_Girls;
    std::string summary, msg, girlName, MatronMsg, MatronWarningMsg;
    int totalGold;
    bool refused;
    Girl* DeadGirl = nullptr;
    
    // WD:  Set processing flag to shift type
    m_Processing_Shift = DayNight;
    
    /*
     *  handle any girls training during this shift
     */
    m_JobManager.do_training( brothel, DayNight );
    
    /*
     *  as for the rest of them...
     */
    while( current )
    {
        //srand(g_Graphics.GetTicks()+(g_Dice%5432));
        totalGold = 0;
        //summary = "";
        refused = false;
        current->m_Pay = 0;
        girlName = current->m_Realname;
        
        /*
         *      ONCE DAILY processing
         *      at start of Day Shift
         */
        if( DayNight == SHIFT_DAY )
        {
            // Remove any dead bodies from last week
            if( current->health() <= 0 )
            {
                DeadGirl = current;
                
                if( current->m_Next ) // If there are more girls to process
                    current = current->m_Next;
                else
                    current = nullptr;
                    
                UpdateAllGirlsStat( brothel, STAT_PCFEAR, 2 );  // increase all the girls fear of the player for letting her die (weather his fault or not)
                UpdateAllGirlsStat( brothel, STAT_PCHATE, 1 );  // increase all the girls hate of the player for letting her die (weather his fault or not)
                
                // Two messages go into the girl queue...
                msg += girlName + " has died from her injuries, the other girls all fear and hate you a little more.";
                DeadGirl->m_Events.AddMessage( msg, IMGTYPE_DEATH, EVENT_DANGER );
                summary += girlName + " has died from her injuries.  Her body will be removed by the end of the week.";
                DeadGirl->m_Events.AddMessage( summary, IMGTYPE_DEATH, EVENT_SUMMARY );
                
                // There is also one global message
                g_MessageQue.AddToQue( msg, 1 );
                
                // I forgot to set msg and summary to empty. D'OH!
                msg = "";
                summary = "";
                
                RemoveGirl( brothel->m_id, DeadGirl );
                DeadGirl = nullptr;
                
                if( current ) // If there are more girls to process
                    continue;
                else
                    break;
            }
            
            current->m_Tort = false;
            
            // Brothel only update for girls accomadation level
            do_food_and_digs( brothel, current );
            
            // update the fetish traits
            g_Girls.CalculateGirlType( current );
            
            // update birthday counter and age the girl
            g_Girls.updateGirlAge( current, true );
            
            // update temp stats
            g_Girls.updateTempStats( current );
            
            // update temp skills
            g_Girls.updateTempSkills( current );
            
            // update temp traits
            g_Girls.updateTempTraits( current );
            
            // handle pregnancy and children growing up
            g_Girls.HandleChildren( current, summary );
            
            // health loss to STD's     NOTE: Girl can die
            g_Girls.updateSTD( current );
            
            // Update happiness due to Traits       NOTE: Girl can die
            g_Girls.updateHappyTraits( current );
            
            //  Update daily stats  Now only runs once per day
            updateGirlTurnBrothelStats( current );
            
            //  Stat Code common to Dugeon and Brothel
            g_Girls.updateGirlTurnStats( current );
            
            // Check for dead girls
            // MYR: Moved this code in with the rest, above
            /*if (current->health() <= 0)
            {
                UpdateAllGirlsStat(brothel, STAT_PCFEAR, 2);    // increase all the girls fear of the player for letting her die (weather his fault or not)
                UpdateAllGirlsStat(brothel, STAT_PCHATE, 1);    // increase all the girls hate of the player for letting her die (weather his fault or not)
                msg = girlName + " has died from her injuries, the other girls all fear and hate you a little more.";
                current->m_Events.AddMessage(msg, IMGTYPE_DEATH, EVENT_DANGER);
                summary += girlName + " has died from her injuries.  Her body will be removed by the end of the week.";
                current->m_Events.AddMessage(summary, IMGTYPE_DEATH, EVENT_SUMMARY);
                current = current->m_Next;
                continue;
            }*/
        }
        
        
        /*
         *      EVERY SHIFT processing
         */
        
        // Sanity check! Don't process dead girls
        // Sanity check part 2: Check that m_Next points to something
        if( current->health() <= 0 )
        {
            if( current->m_Next ) // If there are more girls to process
            {
                current = current->m_Next;
                continue;
            }
            else
            {
                current = nullptr;
                break;
            }
        }
        
        // Girl uses items she has
        g_Girls.UseItems( current );
        
        // Calculate the girls asking price
        g_Girls.CalculateAskPrice( current, true );
        
        /*
         *      JOB PROCESSING
         */
        u_int sw = 0;                       //  Job type
        
        if( current->m_JustGaveBirth )      // if she gave birth, let her rest this week
            sw = JOB_RESTING;
        else
            sw = ( DayNight == SHIFT_DAY ) ? current->m_DayJob : current->m_NightJob;
            
        // do their job
        if( sw != JOB_ADVERTISING )         // advertising is handled earlier, before customer generation
        {
            refused = m_JobManager.JobFunctions[sw]( current, brothel, DayNight, summary );
            
            if( refused )                   // if she refused she still gets tired
                g_Girls.AddTiredness( current );
        }
        
        totalGold += current->m_Pay;
        
        // work out the pay between the house and the girl
        g_Brothels.CalculatePay( brothel, current, sw );
        
        brothel->m_Fame += g_Girls.GetStat( current, STAT_FAME );
        
        // Check for dead girls
        // Avoids all the Warning Messages you get for a dead girl
        // MYR: This has already been done. Commenting it out.
        /*if (current->health() <= 0)
        {
            UpdateAllGirlsStat(brothel, STAT_PCFEAR, 2);    // increase all the girls fear of the player for letting her die (weather his fault or not)
            UpdateAllGirlsStat(brothel, STAT_PCHATE, 1);    // increase all the girls hate of the player for letting her die (weather his fault or not)
            msg = girlName + " has died from her injuries, the other girls all fear and hate you a little more.";
            current->m_Events.AddMessage(msg, IMGTYPE_DEATH, EVENT_DANGER);
            summary += girlName + " has died from her injuries.  Her body will be removed by the end of the week.";
            current->m_Events.AddMessage(summary, IMGTYPE_DEATH, EVENT_SUMMARY);
            current = current->m_Next;
            continue;
        }*/
        
        
        // Runaway, Depression & Drug checking
        if( runaway_check( brothel, current ) == true )
        {
            Girl* temp = current;
            current = current->m_Next;
            g_Brothels.RemoveGirl( brothel->m_id, temp, false );
            g_Brothels.AddGirlToRunaways( temp );
            continue;
        }
        
        /*
         *      MATRON CODE START
         */
        
        // Lets try to compact multiple messages into one.
        MatronMsg = "";
        MatronWarningMsg = "";
        
        bool matron = false;
        
        if( GetNumGirlsOnJob( brothel->m_id, JOB_MATRON, true ) >= 1 || GetNumGirlsOnJob( brothel->m_id, JOB_MATRON, false ) >= 1 )
            matron = true;
            
        if( g_Girls.GetStat( current, STAT_TIREDNESS ) > 80 )
        {
            if( matron )
            {
                if( current->m_PrevNightJob == 255 && current->m_PrevDayJob == 255 )
                {
                    current->m_PrevDayJob = current->m_DayJob;
                    current->m_PrevNightJob = current->m_NightJob;
                    current->m_DayJob = current->m_NightJob = JOB_RESTING;
                    MatronWarningMsg += "Your matron takes " + girlName + " off duty to rest due to her tiredness.\n";
                    //current->m_Events.AddMessage(msg, IMGTYPE_DEATH, EVENT_WARNING);
                }
                else
                {
                    if( ( g_Dice % 100 ) + 1 < 70 )
                    {
                        MatronMsg += "Your matron helps " + girlName + " to relax.\n";
                        //current->m_Events.AddMessage(msg, IMGTYPE_PROFILE, DayNight);
                        g_Girls.UpdateStat( current, STAT_TIREDNESS, -5 );
                    }
                }
            }
            else
                MatronWarningMsg += "CAUTION! This girl desparatly need rest. Give her some free time\n";
                
            //current->m_Events.AddMessage("CAUTION! This girl desparatly need rest. Give her some free time", IMGTYPE_DEATH, EVENT_WARNING);
        }
        
        if( g_Girls.GetStat( current, STAT_HAPPINESS ) < 40 && matron && ( g_Dice % 100 ) + 1 < 70 )
        {
            MatronMsg = "Your matron helps cheer up " + girlName + " after she feels sad.\n";
            //current->m_Events.AddMessage(msg, IMGTYPE_PROFILE, DayNight);
            g_Girls.UpdateStat( current, STAT_HAPPINESS, 5 );
        }
        
        if( g_Girls.GetStat( current, STAT_HEALTH ) < 40 )
        {
            if( matron )
            {
                if( current->m_PrevNightJob == 255 && current->m_PrevDayJob == 255 )
                {
                    current->m_PrevDayJob = current->m_DayJob;
                    current->m_PrevNightJob = current->m_NightJob;
                    current->m_DayJob = current->m_NightJob = JOB_RESTING;
                    MatronWarningMsg += girlName + " is taken off duty by your matron to rest due to her low health.\n";
                    //current->m_Events.AddMessage(msg, IMGTYPE_DEATH, EVENT_WARNING);
                }
                else
                {
                    MatronMsg = "Your matron helps heal " + girlName + ".\n";
                    //current->m_Events.AddMessage(msg, IMGTYPE_PROFILE, DayNight);
                    g_Girls.UpdateStat( current, STAT_HEALTH, 5 );
                }
            }
            else
            {
                MatronWarningMsg = "DANGER " + girlName + "'s health is very low!\nShe must rest or she will die!\n";
                //current->m_Events.AddMessage(msg, IMGTYPE_DEATH, EVENT_DANGER);
            }
        }
        
        
        // MOVED Check for dead girls to before matron code to avoid all the unneeded health warning messages
        
        // Back to work
        if( ( current->m_NightJob == JOB_RESTING && current->m_DayJob == JOB_RESTING ) && ( g_Girls.GetStat( current, STAT_HEALTH ) >= 80 && g_Girls.GetStat( current, STAT_TIREDNESS ) <= 20 ) )
        {
            if(
                ( matron || current->m_PrevDayJob == JOB_MATRON ) // do we have a Matron, or was she the Matron and made herself rest?
                && current->m_PrevDayJob != 255  // 255 = nothing, in other words no previous job stored
                && current->m_PrevNightJob != 255
            )
            {
                g_Brothels.m_JobManager.HandleSpecialJobs( brothel->m_id, current, current->m_PrevDayJob, current->m_DayJob, true );
                
                //current->m_DayJob = current->m_PrevDayJob;  // covered by HandleSpecialJobs above
                if( current->m_DayJob == current->m_PrevDayJob ) // only update night job if day job passed HandleSpecialJobs
                    current->m_NightJob = current->m_PrevNightJob;
                else
                    current->m_DayJob = JOB_RESTING;
                    
                current->m_PrevDayJob = current->m_PrevNightJob = 255;
                MatronMsg += "Matron puts " + girlName + " back to work.\n";
                //current->m_Events.AddMessage(msg, IMGTYPE_PROFILE, DayNight);
            }
            else
            {
                current->m_DayJob = JOB_RESTING;
                MatronWarningMsg += "WARNING " + girlName + " is doing nothing!\n";
                //current->m_Events.AddMessage(msg, IMGTYPE_PROFILE, EVENT_WARNING);
            }
        }
        
        // Now print out the consolodated message
        if( strcmp( MatronMsg.c_str(), "" ) != 0 )
        {
            current->m_Events.AddMessage( MatronMsg, IMGTYPE_PROFILE, DayNight );
            MatronMsg = "";
        }
        
        if( strcmp( MatronWarningMsg.c_str(), "" ) != 0 )
        {
            current->m_Events.AddMessage( MatronWarningMsg, IMGTYPE_PROFILE, EVENT_WARNING );
            MatronWarningMsg = "";
        }
        
        /*
         *      MATRON CODE END
         */
        
        // update girl triggers
        current->m_Triggers.ProcessTriggers();
        
        /*
         *      Summary Messages
         */
        bool sum = true;
        
        if( sw == JOB_RESTING )
            summary += girlName + " was resting so made no money.";
            
        else if( sw == JOB_TRAINING )
            sum = false;
            
        else if( sw == JOB_MATRON && DayNight == SHIFT_NIGHT )
            summary += girlName + " continued to help the other girls throughout the night.";
            
        else if( sw == JOB_ADVERTISING )
            sum = false;
            
        // WD:  No night shift sunnary message needed for Torturer job
        else if( sw == JOB_TORTURER && DayNight == SHIFT_NIGHT )
            sum = false;
            
        // WD:  Bad girl did not work. Moved from cJobManager::Preprocessing()
        else if( refused )
            summary += girlName + " refused to work so made no money.";
            
        // WD:  Only do summary messages if there is income to report
        else if( totalGold > 0 )
        {
            summary += girlName + " earned a total of ";
            summary += toString( totalGold );
            summary += " gold";
//          if(sw == JOB_MATRON)

            // WD: Job Paid by player
            if( m_JobManager.is_job_Paid_Player( sw ) )
                summary += " directly from you. She gets to keep it all.";
            else if( current->house() <= 0 )
                summary += " and she gets to keep it all.";
            else
            {
                summary += ", you keep ";
                summary += toString( current->m_Stats[STAT_HOUSE] );
                summary += "%. ";
            }
        }
        
        // WD:  No Income today
        else if( totalGold == 0 )
            summary += girlName + " made no money.";
            
#if 1                                                               // WD: Income Loss Sanity Checking
        else if( totalGold < 0 )
        {
            summary += "ERROR: She has a loss of ";
            summary += toString( totalGold );
            summary += " gold";
            summary += "\n\n Please report this to the Pink Petal Devloment Team at http://pinkpetal.co.cc";
            current->m_Events.AddMessage( summary, IMGTYPE_PROFILE, EVENT_DEBUG );
            sum = false;
        }
        
#endif
        
        if( sum )
            current->m_Events.AddMessage( summary, IMGTYPE_PROFILE, EVENT_SUMMARY );
            
        summary = "";
        
        // Level the girl up if nessessary
        if( g_Girls.GetStat( current, STAT_EXP ) == 255 )
            g_Girls.LevelUp( current );
            
        // Myr: Natural healing. This amounts to 2% health/tiredness per shift and is not designed in any
        //      way to replace any healing item or mechanism in the game.
        //      I'm specifically not using g_Girls.UpdateStat as I don't want the modifiers for toughness
        //      and fragile to change the base 2% (Fragile would change 2% health gain to 1% health damage,
        //      which makes no sense and tough would change 2% health gain to 4% (8% per turn) which is
        //      IMO too high.)
        current->m_Stats[STAT_HEALTH] += 2;
        
        if( current->m_Stats[STAT_HEALTH] > 100 )
            current->m_Stats[STAT_HEALTH] = 100;
            
        // Wow, this tiredness code causes the game to go nuts! Commented out for now
        //current->m_Stats[STAT_TIREDNESS] = current->m_Stats[STAT_TIREDNESS] - 2;
        //if (current->m_Stats[STAT_TIREDNESS] < 0)
        //  current->m_Stats[STAT_TIREDNESS] = 0;
        
        // Myr: Automate the use of a number of different items. See the function itself for more comments.
        //      Enabled or disabled based on config option.
        cConfig cfg;
        
        if( cfg.initial.auto_use_items() )
            UsePlayersItems( current );
            
        // Process next girl
        current = current->m_Next;
    }
    
    // WD: Finished Processing Shift set flag
    m_Processing_Shift = -1;
}

void BrothelManager::UsePlayersItems( Girl* cur )
{
    int has = 0, has2 = 0, Die = 0, PolishCount = 0;
    
    /* Automatic item use - to stop the monotonous work.
       (I started writing this for my test game where I had 6 brothels with
       125+ girls in each. 16 of them were full time catacombs explorers.)
    
       Food type items are forced. Actual pieces of equipment are not.
       The players equipment choices should always be respected.
    
       There are a number of things this function specifically DOES NOT do:
       1. Use skill raising items.
       2. Cure diseases like aids and syphilus.
       3. Cure addictions like shroud and fairy dust.
       4. Use temporary items.
       5. Use items related to pregnancy, insemenation or children
    
       I should qualify this by saying, "It doesn't directly raise stats, cure
       diseases and addictions." They can happen indirectly as a piece of equipment
       equipped for a stat boost or trait may also raise skills. Similarily a
       item used to cure some condition (like an Elixir of Ultimate Regeneration
       curing one-eye or scars)  may also cure a disease or addiction as well.
    
       The way this is currently written it shouldn't be released as part
       of the game. It makes too many choices for the player. Perhaps we can
       make it into a useful game function somehow. Regardless, this can be
       disabled by commenting out a single line in UpdateGirls.
    */
    
    // ------------ Part 1: Stats -------------
    
    # pragma region automation_stats
    
    // Health
    
    // Healing items are wasted on constructs as the max. 4% applies to both damage and
    // healing
    has = g_Brothels.HasItem( "Healing Salve (L)" );
    
    if( g_Girls.GetStat( cur, STAT_HEALTH ) <= 25 && !g_Girls.HasTrait( cur, "Construct" ) && has != -1 )
        AutomaticFoodItemUse( cur, has, "Used a large healing salve to stay healthy." );
        
    has = g_Brothels.HasItem( "Healing Salve (M)" );
    
    if( g_Girls.GetStat( cur, STAT_HEALTH ) <= 50 && !g_Girls.HasTrait( cur, "Construct" ) && has != -1 )
        AutomaticFoodItemUse( cur, has, "Used a medium healing salve to stay healthy." );
        
    has = g_Brothels.HasItem( "Healing Salve (S)" );
    
    if( g_Girls.GetStat( cur, STAT_HEALTH ) <= 75 && !g_Girls.HasTrait( cur, "Construct" ) && has != -1 )
        AutomaticFoodItemUse( cur, has, "Used a small healing salve to stay healthy." );
        
    // Tiredness/fatigue
    has = g_Brothels.HasItem( "Incense of Serenity (L)" );
    
    if( g_Girls.GetStat( cur, STAT_TIREDNESS ) >= 75 && !g_Girls.HasTrait( cur, "Construct" ) && has != -1 )
        AutomaticFoodItemUse( cur, has, "Used a large incense of serenity to stay awake." );
        
    has = g_Brothels.HasItem( "Incense of Serenity (M)" );
    
    if( g_Girls.GetStat( cur, STAT_TIREDNESS ) >= 50 && !g_Girls.HasTrait( cur, "Construct" ) && has != -1 )
        AutomaticFoodItemUse( cur, has, "Used a medium incense of serenity to stay awake." );
        
    has = g_Brothels.HasItem( "Incense of Serenity (S)" );
    
    if( g_Girls.GetStat( cur, STAT_TIREDNESS ) >= 25 && !g_Girls.HasTrait( cur, "Construct" ) && has != -1 )
        AutomaticFoodItemUse( cur, has, "Used a small incense of serenity to stay awake." );
        
    // Mana
    
    // Set threshold at 20 as that is what is required to charm a customer to sleep with a girl
    has = g_Brothels.HasItem( "Mana Crystal" );
    
    if( g_Girls.GetStat( cur, STAT_MANA ) < 20 && has != -1 )
    {
        Die = g_Dice % 20 + 1;
        
        if( Die == 12 ) // Share them around
            AutomaticFoodItemUse( cur, has, "Used a mana crystal to restore 25 mana." );
    }
    
    has = g_Brothels.HasItem( "Eldritch Cookie" );
    
    if( g_Girls.GetStat( cur, STAT_MANA ) < 20 && has != -1 )
    {
        Die = g_Dice % 20 + 1;
        
        if( Die == 13 ) // Share them around
            AutomaticFoodItemUse( cur, has, "Used an eldritch cookie to restore 30 mana." );
    }
    
    has = g_Brothels.HasItem( "Mana Potion" );
    
    if( g_Girls.GetStat( cur, STAT_MANA ) < 20 && has != -1 )
    {
        Die = g_Dice % 20 + 1;
        
        if( Die == 15 ) // Share them around
            AutomaticFoodItemUse( cur, has, "Used a mana potion to restore 100 mana." );
    }
    
    // Libido - ordered big to small
    
    // Succubus Milk [100 pts]
    has = g_Brothels.HasItem( "Succubus Milk" );
    
    if( g_Girls.GetStat( cur, STAT_LIBIDO ) < 5 && has != -1 ) // Lower threshold
        AutomaticFoodItemUse( cur, has, "Used succubus milk to restore 100 libido." );
        
    // Sinspice [75 pts]
    has = g_Brothels.HasItem( "Sinspice" );
    
    if( g_Girls.GetStat( cur, STAT_LIBIDO ) < 10 && has != -1 )
        AutomaticFoodItemUse( cur, has, "Used sinspice to restore 75 libido." );
        
    //Empress' New Clothes [50 pts] (Piece of equipment)  (This is a tossup between charisma & libido)
    has = g_Brothels.HasItem( "Empress' New Clothes" );
    
    if( g_Girls.GetStat( cur, STAT_LIBIDO ) <= 10 && has != -1 )
        AutomaticItemUse( cur, has, "You had her put on the empress' new clothes to get her libido up." );
        
    // Red Rose Extravaganza [50 pts?]
    has = g_Brothels.HasItem( "Red Rose Extravaganza" );
    
    if( g_Girls.GetStat( cur, STAT_LIBIDO ) < 10 && has != -1 )
        AutomaticFoodItemUse( cur, has, "Gave her a red rose extravaganza to get her libido going again." );
        
    // Ring of the Horndog [50 pts] (Piece of equipment)
    has = g_Brothels.HasItem( "Ring of the Horndog" );
    
    if( g_Girls.GetStat( cur, STAT_LIBIDO ) <= 10 && g_Girls.HasItem( cur, "Minor Ring of the Horndog" ) == -1
            && g_Girls.HasItem( cur, "Ring of the Horndog" ) == -1 && g_Girls.HasItem( cur, "Organic Lingerie" ) == -1 && has != -1 )
        AutomaticItemUse( cur, has, "You had her equip a ring of the horndog to better serve her customers. (Libido up.)" );
        
    // Gemstone Dress [42 pts] (Piece of equipment)
    has = g_Brothels.HasItem( "Gemstone Dress" );
    
    if( g_Girls.GetStat( cur, STAT_LIBIDO ) <= 10 && has != -1 )
        AutomaticItemUse( cur, has, "She put on a gemstone dress for that million-dollar feeling. (Libido up.)" );
        
    // Silken Dress [34 pts] (Piece of equipment)
    has = g_Brothels.HasItem( "Silken Dress" );
    
    if( g_Girls.GetStat( cur, STAT_LIBIDO ) <= 10 && has != -1 )
        AutomaticItemUse( cur, has, "She put on a silken dress to better slide with her customers. (Libido up.)" );
        
    // Minor Ring of the Horndog [30 pts] (Piece of equipment)
    has = g_Brothels.HasItem( "Minor Ring of the Horndog" );
    
    if( g_Girls.GetStat( cur, STAT_LIBIDO ) <= 10 && g_Girls.HasItem( cur, "Minor Ring of the Horndog" ) == -1
            && g_Girls.HasItem( cur, "Ring of the Horndog" ) == -1  && g_Girls.HasItem( cur, "Organic Lingerie" ) == -1 && has != -1 )
        AutomaticItemUse( cur, has, "She was lookin a little listless so you had her equip a minor ring of the horndog. (Libido up.)" );
        
    // Velvet Dress [34 pts] (Piece of equipment)
    has = g_Brothels.HasItem( "Velvet Dress" );
    
    if( g_Girls.GetStat( cur, STAT_LIBIDO ) <= 10 && has != -1 )
        AutomaticItemUse( cur, has, "She put on a velvet dress to feel even more sexy. (Libido up.)" );
        
    // Designer Lingerie [20 pts] (Piece of equipment)
    has = g_Brothels.HasItem( "Designer Lingerie" );
    
    if( g_Girls.GetStat( cur, STAT_LIBIDO ) <= 10 && has != -1 )
        AutomaticItemUse( cur, has, "She put on designer lingerie to feel more at home. (Libido up.)" );
        
    // Charisma
    
    //Ring of Charisma [50 pts] (Piece of equipment)
    has = g_Brothels.HasItem( "Ring of Charisma" );
    
    if( g_Girls.GetStat( cur, STAT_CHARISMA ) <= 50 && g_Girls.HasItem( cur, "Ring of Charisma" ) == -1 && has != -1 )
        AutomaticItemUse( cur, has, "You had her put on a ring of charisma to overcome her speaking difficulties." );
        
    // Minor Ring of Charisma [30 pts] (Piece of equipment)
    has = g_Brothels.HasItem( "Minor Ring of Charisma" );
    
    if( g_Girls.GetStat( cur, STAT_CHARISMA ) <= 70 && g_Girls.HasItem( cur, "Minor Ring of Charisma" ) == -1 && has != -1 )
        AutomaticItemUse( cur, has, "Put on a minor ring of charisma." );
        
    // Beauty
    
    // Ring of Beauty [50 pts] (Piece of equipment)
    has = g_Brothels.HasItem( "Ring of Beauty" );
    
    if( g_Girls.GetStat( cur, STAT_BEAUTY ) <= 50 && g_Girls.HasItem( cur, "Ring of Beauty" ) == -1 && has != -1 )
        AutomaticItemUse( cur, has, "You had her put on a ring of beauty to overcome her ugly-stick disadvantage." );
        
    // Minor Ring of Beauty [30 pts] (Piece of equipment)
    has = g_Brothels.HasItem( "Minor Ring of Beauty" );
    
    if( g_Girls.GetStat( cur, STAT_BEAUTY ) <= 70 && g_Girls.HasItem( cur, "Minor Ring of Beauty" ) == -1 && has != -1 )
        AutomaticItemUse( cur, has, "She put on a minor ring of beauty to cover some flaws." );
        
    // Rainbow Ring [15 pts] (Piece of equipment)
    has = g_Brothels.HasItem( "Rainbow Ring" );
    
    if( g_Girls.GetStat( cur, STAT_BEAUTY ) <= 85 && g_Girls.HasItem( cur, "Rainbow Ring" ) == -1 && has != -1 )
        AutomaticItemUse( cur, has, "She put on a rainbow ring, to match her rainbow personality." );
        
    // Happiness - ordered from big values to small
    
    // Heaven-and-Earth Cake [100 pts]
    has = g_Brothels.HasItem( "Heaven-and-Earth Cake" );
    
    if( g_Girls.GetStat( cur, STAT_HAPPINESS ) <= 10 && has != -1 )
        AutomaticFoodItemUse( cur, has, "Had a heaven-and-earth cake to stave off serious depression." );
        
    // Eldritch cookie [70 pts]
    has = g_Brothels.HasItem( "Eldritch Cookie" );
    
    if( g_Girls.GetStat( cur, STAT_HAPPINESS ) <= 30 && has != -1 )
        AutomaticFoodItemUse( cur, has, "Had an eldritch cookie to improve her mood." );
        
    // Expensive Chocolates [50 pts]
    has = g_Brothels.HasItem( "Expensive Chocolates" );
    
    if( g_Girls.GetStat( cur, STAT_HAPPINESS ) <= 50 && has != -1 )
        AutomaticFoodItemUse( cur, has, "Had some expensive chocolates to improve her mood." );
        
    // Apple Tart [30 pts]
    has = g_Brothels.HasItem( "Apple Tart" );
    
    if( g_Girls.GetStat( cur, STAT_HAPPINESS ) <= 70 && has != -1 )
        AutomaticFoodItemUse( cur, has, "Had an apple tart to improve her mood." );
        
    // Honeypuff Scones [30 pts]
    has = g_Brothels.HasItem( "Honeypuff Scones" );
    
    if( g_Girls.GetStat( cur, STAT_HAPPINESS ) <= 70 && has != -1 )
        AutomaticFoodItemUse( cur, has, "Had a honeypuff scone for lunch." );
        
    // Fancy breath mints [10 pts]
    has = g_Brothels.HasItem( "Fancy Breath Mints" );
    
    if( g_Girls.GetStat( cur, STAT_HAPPINESS ) <= 90 && has != -1 )
        AutomaticFoodItemUse( cur, has, "Had fancy breath mints. (Why not? They were lying around.)" );
        
    // Exotic Bouquet [10 pts]
    has = g_Brothels.HasItem( "Exotic Bouquet" );
    
    if( g_Girls.GetStat( cur, STAT_HAPPINESS ) <= 90 && has != -1 )
        AutomaticFoodItemUse( cur, has, "You gave her an exotic bouquet for work well done." );
        
    // Wild Flowers [5 pts]
    has = g_Brothels.HasItem( "Wild Flowers" );
    
    if( g_Girls.GetStat( cur, STAT_HAPPINESS ) <= 95 && has != -1 )
        AutomaticFoodItemUse( cur, has, "You gave her some wild flowers." );
        
    // Age
    
    //Do this before boobs b/c lolly wand makes them small
    // My arbitrary rule is, once they hit 30, make 'em young again.
    
    // To prevent using an elixir, then a wand, set an arbitrary upper age limit of 35 for elixirs
    has = g_Brothels.HasItem( "Elixir of Youth" );
    
    if( ( g_Girls.GetStat( cur, STAT_AGE ) >= 30 ) && ( g_Girls.GetStat( cur, STAT_AGE ) <= 35 ) && ( has != -1 ) )
        AutomaticFoodItemUse( cur, has, "Used a elixir of youth to remove ten years of age." );
        
    has = g_Brothels.HasItem( "Lolita Wand" );
    
    if( g_Girls.GetStat( cur, STAT_AGE ) >= 30 && g_Girls.GetStat( cur, STAT_AGE ) <= 80 && has != -1 )
        AutomaticFoodItemUse( cur, has, "Used a lolita wand to become seventeen again." );
        
    // XP: Nuts & tomes & mangos of knowledge, etc...
    
    // 25 xp
    has = g_Brothels.HasItem( "Nut of Knowledge" );
    
    if( g_Girls.GetStat( cur, STAT_EXP ) <= 230 && g_Girls.GetStat( cur, STAT_LEVEL ) < 255 && has != -1 )
    {
        Die = g_Dice % 50;   // Spread them around
        
        if( Die == 5 )
            AutomaticFoodItemUse( cur, has, "Used a small nut of knowledge." );
    }
    
    // 100 xp
    has = g_Brothels.HasItem( "Mango of Knowledge" );
    
    if( g_Girls.GetStat( cur, STAT_EXP ) <= 155 && g_Girls.GetStat( cur, STAT_LEVEL ) < 255 && has != -1 )
    {
        Die = g_Dice % 30;
        
        if( Die == 9 )
            AutomaticFoodItemUse( cur, has, "She ate a mango of knowledge." );
    }
    
    // 200 xp
    has = g_Brothels.HasItem( "Watermelon of Knowledge" );
    
    if( g_Girls.GetStat( cur, STAT_EXP ) <= 55 && g_Girls.GetStat( cur, STAT_LEVEL ) < 255 && has != -1 )
    {
        Die = g_Dice % 30;
        
        if( Die == 9 )
            AutomaticFoodItemUse( cur, has, "She had a watermelon of knowledge for lunch." );
    }
    
    // Constitution (Items in reverse order. That is, the items offering the largest increases are first)
    
    // Ring of the Schwarzenegger [50 pts] (Piece of equipment)
    has = g_Brothels.HasItem( "Ring of the Schwarzenegger" );
    
    if( g_Girls.GetStat( cur, STAT_CONSTITUTION ) <= 50 && has != -1 )
        AutomaticItemUse( cur, has, "She put on a Ring of the Schwarzenegger for the constitution boost." );
        
    // Bracer of Toughness [40 pts] (Piece of equipment)
    has = g_Brothels.HasItem( "Bracer of Toughness" );
    
    if( g_Girls.GetStat( cur, STAT_CONSTITUTION ) <= 60 && has != -1 )
        AutomaticItemUse( cur, has, "She put on a bracer of toughness for the constitution boost." );
        
    // Minor Ring of the Schwarzenegger [30 pts] (Piece of equipment)
    has = g_Brothels.HasItem( "Minor Ring of the Schwarzenegger" );
    
    if( g_Girls.GetStat( cur, STAT_CONSTITUTION ) <= 70 && has != -1 )
        AutomaticItemUse( cur, has, "She put on a Minor Ring of the Schwarzenegger for the constitution boost." );
        
    // Necklace of Pain Reversal [25 pts net: +40 for masochist -15 on necklace] (Piece of equipment)
    has = g_Brothels.HasItem( "Necklace of Pain Reversal" );
    
    if( g_Girls.GetStat( cur, STAT_CONSTITUTION ) <= 75 && !g_Girls.HasTrait( cur, "Masochist" ) && has != -1 )
        AutomaticItemUse( cur, has, "You had her put on a necklace of pain reversal for the constitution boost." );
        
    // Tiger Leotard [20 pts] (Piece of equipment)
    has = g_Brothels.HasItem( "Tiger Leotard" );
    
    if( g_Girls.GetStat( cur, STAT_CONSTITUTION ) <= 80 && has != -1 )
        AutomaticItemUse( cur, has, "She put on a tiger leotard to feel it's strength and power." );
        
    // Manual of health [10 pts] (Piece of equipment, but slotless)
    // Lets be reasonable and only allow only one of each slotless item to be given to a girl.
    // (Having 8 stripper poles in a girl's inventory looks silly IMO.)
    has = g_Brothels.HasItem( "Manual of Health" );
    
    if( g_Girls.GetStat( cur, STAT_CONSTITUTION ) <= 90 && g_Girls.HasItem( cur, "Manual of Health" ) == -1 && has != -1 )
        AutomaticSlotlessItemUse( cur, has, "You gave her a manual of health to read." );
        
    // Free Weights [10 pts] (Piece of equipment, but slotless)
    has = g_Brothels.HasItem( "Free Weights" );
    
    if( g_Girls.GetStat( cur, STAT_CONSTITUTION ) <= 90 && g_Girls.HasItem( cur, "Free Weights" ) == -1 && has != -1 )
        AutomaticSlotlessItemUse( cur, has, "You gave her free weights to work with." );
        
    // Stripper Pole [5 pts] (Piece of equipment, but slotless)
    has = g_Brothels.HasItem( "Stripper Pole" );
    
    if( g_Girls.GetStat( cur, STAT_CONSTITUTION ) <= 95 && g_Girls.HasItem( cur, "Stripper Pole" ) == -1 && has != -1 )
        AutomaticSlotlessItemUse( cur, has, "You gave her a stripper pole to practice with." );
        
    // Obedience
    
    // Necklace of Control (piece of equipment)
    has = g_Brothels.HasItem( "Necklace of Control" );
    
    if( g_Girls.GetStat( cur, STAT_OBEDIENCE ) <= 10 && has != -1 )
        AutomaticItemUse( cur, has, "Her obedience is a problem so you had her put on a necklace of control." );
        
    has = g_Brothels.HasItem( "Disguised Slave Band" );
    
    if( g_Girls.GetStat( cur, STAT_OBEDIENCE ) <= 50 && has != -1 )
        AutomaticItemUse( cur, has, "You had her put on a disguised slave band, claiming it was something else." );
        
    has = g_Brothels.HasItem( "Slave Band" );
    
    if( g_Girls.GetStat( cur, STAT_OBEDIENCE ) <= 50 && has != -1 )
        AutomaticItemUse( cur, has, "You dealth with her obedience problems by forcing her to wear a slave band." );
        
    has = g_Brothels.HasItem( "Willbreaker Spice" );
    
    if( g_Girls.GetStat( cur, STAT_OBEDIENCE ) <= 90 && has != -1 )
        AutomaticFoodItemUse( cur, has, "You slipped some willbreaker spice in to her food." );
        
    #pragma endregion automation_stats
    
    // ---------- Part 2: Traits ----------------
    
    # pragma region automation_traits
    
    // Perfection. This is an uber-valuable I put in. Ideally it should be Catacombs01, not Catacombs15.
    // It changes so many traits that it's hard to decide on a rule. In the end I kept it simple.
    // (Players will justifiably hate me if I made this decision for them.)
    // Do this first as it covers/replaces 90% of what follows
    has = g_Brothels.HasItem( "Perfection" );
    
    if( cur->m_NumTraits <= 8 && has != -1 )
        AutomaticFoodItemUse( cur, has, "Used perfection to become a near perfect being." );
        
    // Tough
    
    // Aoshima beef
    has = g_Brothels.HasItem( "Aoshima BEEF!!" );
    
    if( !g_Girls.HasTrait( cur, "Tough" ) && has != -1 )
        AutomaticFoodItemUse( cur, has, "Bulked up on Aoshima Beef to get the tough trait." );
        
    // Oiran Dress (Piece of equipment)
    has = g_Brothels.HasItem( "Oiran Dress" );
    
    if( !g_Girls.HasTrait( cur, "Tough" ) && has != -1 )
        AutomaticItemUse( cur, has, "Put on an Oiran Dress." );
        
    // Nymphomaniac
    
    // Do this before quick learner b/c taking the shroud cola gives the girl the slow learner trait
    has = g_Brothels.HasItem( "Shroud Cola" );
    has2 = g_Brothels.HasItem( "Cure for Shroud Addiction" );
    
    if( !g_Girls.HasTrait( cur, "Nymphomaniac" ) && ( has != -1 && has2 != -1 ) )
    {
        // If one succeeds, the other should too
        // Note the order is important here: Shroud cola has to be first
        AutomaticFoodItemUse( cur, has,
                              "You had her down a shround cola for the nymphomaniac side-effect.\n\n Unfortunately she also gains the slow-learner trait." );
        AutomaticFoodItemUse( cur, has2, "You had her take the shroud addiction cure." );
    }
    
    // Quick learner
    
    // Scroll of transcendance
    has = g_Brothels.HasItem( "Scrolls of Transcendance" );
    
    if( !g_Girls.HasTrait( cur, "Quick Learner" ) && !g_Girls.HasTrait( cur, "Optimist" ) && has != -1 )
        AutomaticFoodItemUse( cur, has, "Read a Scroll of Transcendence to gain the quick learner and optimist traits." );
        
    // Book of enlightenment
    has = g_Brothels.HasItem( "Book of Enlightenment" );
    
    if( !g_Girls.HasTrait( cur, "Quick Learner" ) && has != -1 )
        AutomaticFoodItemUse( cur, has, "Read a book of enlightenment for the quick learner trait." );
        
    // Ring of Enlightenment
    has = g_Brothels.HasItem( "Ring of Enlightenment" );
    
    if( !g_Girls.HasTrait( cur, "Quick Learner" ) && has != -1 )
        AutomaticItemUse( cur, has, "Put on a ring of enlightenment for the quick learner trait." );
        
    // Amulet of the Cunning Linguist
    has = g_Brothels.HasItem( "Amulet of the Cunning Linguist" );
    
    if( !g_Girls.HasTrait( cur, "Quick Learner" ) && has != -1 )
        AutomaticItemUse( cur, has, "Put on an amulet of the cunning linguist for the quick learner trait." );
        
    // Optimist: Good fortune, leprechaun biscuit, chatty flowers, etc...
    
    // Good Fortune
    has = g_Brothels.HasItem( "Good Fortune" );
    
    if( !g_Girls.HasTrait( cur, "Optimist" ) && has != -1 )
        AutomaticFoodItemUse( cur, has, "Read a good fortune and feels more optimistic for it." );
        
    // Leprechaun Biscuit
    has = g_Brothels.HasItem( "Leprechaun Biscuit" );
    
    if( !g_Girls.HasTrait( cur, "Optimist" ) && has != -1 )
        AutomaticFoodItemUse( cur, has, "Had a leprechaun biscuit and feels more optimistic for it." );
        
    // Chatty Flowers
    has = g_Brothels.HasItem( "Chatty Flowers" );
    
    if( !g_Girls.HasTrait( cur, "Optimist" ) && has != -1 )
        AutomaticFoodItemUse( cur, has, "Talked with the chatty flowers and feels more optimistic for it." );
        
    // Glass shoes (piece of equipment)
    has = g_Brothels.HasItem( "Glass Shoes" );
    
    if( !g_Girls.HasTrait( cur, "Optimist" ) && g_Girls.HasItem( cur, "Sandals of Mercury" ) == -1 && has != -1 )
        AutomaticItemUse( cur, has, "Slipped on glass shoes for the optimist trait." );
        
    // Elegant (Obsidian Choker, piece of equipment)
    
    has = g_Brothels.HasItem( "Obsidian Choker" );
    
    if( !g_Girls.HasTrait( cur, "Elegant" ) && has != -1 )
        AutomaticItemUse( cur, has, "Put on an obsidian choker for the elegant trait." );
        
    // Fleet of foot (Sandals of Mercury, piece of equipment)
    
    has = g_Brothels.HasItem( "Sandals of Mercury" );
    
    if( !g_Girls.HasTrait( cur, "Fleet of Foot" ) && g_Girls.HasItem( cur, "Glass Shoes" ) == -1 && has != -1 )
        AutomaticItemUse( cur, has, "Put on Sandals of Mercury for the fleet of foot trait." );
        
    // Fast Orgasms & Nymphomaniac (Organic Lingerie, piece of equipment)
    
    has = g_Brothels.HasItem( "Organic Lingerie" );
    
    if( !g_Girls.HasTrait( cur, "Fast Orgasms" ) && !g_Girls.HasTrait( cur, "Nymphomaniac" ) && has != -1 )
        AutomaticItemUse( cur, has, "You had her wear organic lingerie." );
        
    // Fast Orgasms (Ring of Pleasure, piece of equipment)
    
    has = g_Brothels.HasItem( "Ring of Pleasure" );
    
    if( !g_Girls.HasTrait( cur, "Fast Orgasms" ) && has != -1 )
        AutomaticItemUse( cur, has, "You had her put on a ring of pleasure for the fast orgasms trait." );
        
    // Lets try and cure mind fucked & retarted
    // The amulet of the sex elemental gives you the mind fucked trait. It can be "cured" until the amulet is taken off and put on again.
    // Regardless, we'll not try to cure the amulet case.
    has = g_Brothels.HasItem( "Refined Mandragora Extract" );
    
    if( ( ( g_Girls.HasTrait( cur, "Mind Fucked" ) && g_Girls.HasItem( cur, "Amulet of the Sex Elemental" ) == -1 )
            || g_Girls.HasTrait( cur, "Retarded" ) ) && has != -1 )
        AutomaticFoodItemUse( cur, has, "You had her use refined mandragora extract to remove mental damage." );
        
    // Malformed
    
    has = g_Brothels.HasItem( "Elixir of Ultimate Regeneration" );
    
    if( g_Girls.HasTrait( cur, "Malformed" ) && has != -1 )
        AutomaticFoodItemUse( cur, has, "Used an elixir of ultimate regeneration to cure her malformities." );
        
    // Tsundere & yandere
    
    has = g_Brothels.HasItem( "Attitude Reajustor" );
    
    if( ( g_Girls.HasTrait( cur, "Yandere" ) || g_Girls.HasTrait( cur, "Tsundere" ) ) && has != -1 )
        AutomaticFoodItemUse( cur, has, "You had her take an attitude reajustor pill." );
        
    // Eyes
    
    has = g_Brothels.HasItem( "Eye Replacement Candy" );
    
    if( ( g_Girls.HasTrait( cur, "One Eye" ) || g_Girls.HasTrait( cur, "Eye Patch" ) ) && has != -1 )
        AutomaticFoodItemUse( cur, has, "Used an eye replacement candy to restore her eye." );
        
    // Last ditch eye check.  Use the big guns if you don't have anything else.
    has = g_Brothels.HasItem( "Elixir of Ultimate Regeneration" );
    
    if( ( g_Girls.HasTrait( cur, "One Eye" ) || g_Girls.HasTrait( cur, "Eye Patch" ) ) && has != -1 )
        AutomaticFoodItemUse( cur, has, "Used an elixir of ultimate regeneration to restore her eye." );
        
    // Scars - start with the least powerful cures and work up
    has = g_Brothels.HasItem( "Oil of Lesser Scar Removing" );
    
    if( ( g_Girls.HasTrait( cur, "Small Scars" ) || g_Girls.HasTrait( cur, "Cool Scars" ) ) && has != -1 )
        AutomaticFoodItemUse( cur, has, "Used an oil of lesser scar removal to remove work-related damage." );
        
    has = g_Brothels.HasItem( "Oil of Greater Scar Removing" );
    
    if( ( g_Girls.HasTrait( cur, "Small Scars" ) || g_Girls.HasTrait( cur, "Cool Scars" ) || g_Girls.HasTrait( cur, "Horrific Scars" ) ) && has != -1 )
        AutomaticFoodItemUse( cur, has, "Used an oil of greater scar removal to remove her scars." );
        
    has = g_Brothels.HasItem( "Elixir of Ultimate Regeneration" );
    
    if( ( g_Girls.HasTrait( cur, "Small Scars" ) || g_Girls.HasTrait( cur, "Cool Scars" ) || g_Girls.HasTrait( cur, "Horrific Scars" ) ) && has != -1 )
        AutomaticFoodItemUse( cur, has, "Used an elixir of ultimate regeneration to remove her scars." );
        
    // Big boobs
    
    has = g_Brothels.HasItem( "Oil of Extreme Breast Growth" );
    
    if( !g_Girls.HasTrait( cur, "Big Boobs" ) && !g_Girls.HasTrait( cur, "Abnormally Large Boobs" ) && has != -1 )
        AutomaticFoodItemUse( cur, has, "She uses an oil of extreme breast growth to gain the abnormally large boobs trait." );
        
    has = g_Brothels.HasItem( "Oil of Greater Breast Growth" );
    
    if( !g_Girls.HasTrait( cur, "Big Boobs" ) && !g_Girls.HasTrait( cur, "Abnormally Large Boobs" ) && has != -1 )
        AutomaticFoodItemUse( cur, has, "She uses an oil of greater breast growth to gain the big boobs trait." );
        
    // Nipple Rings of Pillowy Softness (piece of [ring slot] equipment)
    has = g_Brothels.HasItem( "Nipple Rings of Pillowy Softness" );
    
    if( !g_Girls.HasTrait( cur, "Big Boobs" ) && !g_Girls.HasTrait( cur, "Abnormally Large Boobs" ) && has != -1 )
        AutomaticSlotlessItemUse( cur, has, "You had her put on a nipple rings of pillowy softness." );
        
    // Nipple Rings of Breast Expansion, (piece of [ring slot] equipment)
    has = g_Brothels.HasItem( "Nipple Rings of Breast Expansion" );
    
    if( !g_Girls.HasTrait( cur, "Big Boobs" ) && !g_Girls.HasTrait( cur, "Abnormally Large Boobs" ) && has != -1 )
        AutomaticItemUse( cur, has, "You had her put on nipple rings of breast expansion for the big boobs trait." );
        
    // Polish
    has = g_Brothels.HasItem( "Polish" );
    
    if( has != -1 )
    {
        // If the girl doesn't have 4 of these 5 traits she will use polish
        if( !g_Girls.HasTrait( cur, "Good Kisser" ) )
            PolishCount++;
            
        if( !g_Girls.HasTrait( cur, "Great Figure" ) )
            PolishCount++;
            
        if( !g_Girls.HasTrait( cur, "Great Arse" ) )
            PolishCount++;
            
        if( !g_Girls.HasTrait( cur, "Long Legs" ) )
            PolishCount++;
            
        if( !g_Girls.HasTrait( cur, "Puffy Nipples" ) )
            PolishCount++;
            
        if( PolishCount >= 4 )
            AutomaticFoodItemUse( cur, has, "Used polish to make herself more attractive to clients." );
    }
    
    // Masochist
    
    // Put this at the bottom as there are better neck slot items that could be equipped above
    // Unlike the case of raising the constitution score in part one, we're only concerned with the trait here
    has = g_Brothels.HasItem( "Necklace of Pain Reversal" );
    
    if( !g_Girls.HasTrait( cur, "Masochist" ) && has != -1 )
        AutomaticItemUse( cur, has, "You have this thing for masochism, so you had her put on a necklace of pain reversal." );
        
    // Iron Will
    
    // Disguised Slave band (piece of equipment)
    // (Statuses like 'controlled' on the Disguised Slave Band (amongst others) don't appear to do anything.)
    has = g_Brothels.HasItem( "Disguised Slave Band" );
    
    if( g_Girls.HasTrait( cur, "Iron Will" ) && has != -1 )
        AutomaticItemUse( cur, has, "Her iron will is a problem so you had her put on a disguised slave band, claiming it was something else." );
        
    has = g_Brothels.HasItem( "Slave Band" );
    
    if( g_Girls.HasTrait( cur, "Iron Will" ) && has != -1 )
        AutomaticItemUse( cur, has, "You dealth with her iron will by forcing her to wear a slave band." );
        
    // Necklace of Control (piece of equipment)
    has = g_Brothels.HasItem( "Necklace of Control" );
    
    if( g_Girls.HasTrait( cur, "Iron Will" ) && has != -1 )
        AutomaticItemUse( cur, has, "Her iron will is a problem so you had her put on a necklace of control." );
        
    # pragma endregion automation_traits
    
}

bool BrothelManager::AutomaticItemUse( Girl* girl, int InvNum, std::string message )
{
    int EquipSlot = -1;
    
    EquipSlot = g_Girls.AddInv( girl, m_Inventory[InvNum] );
    
    if( EquipSlot != -1 )
    {
        if( g_InvManager.equip_singleton_ok( girl, EquipSlot, false ) ) // Don't force equipment
        {
            RemoveItemFromInventoryByNumber( InvNum ); // Remove from general inventory
            g_InvManager.Equip( girl, EquipSlot, false );
            girl->m_Events.AddMessage( message, IMGTYPE_PROFILE, EVENT_WARNING );
            return true;
            
        }
        else
        {
            g_Girls.RemoveInvByNumber( girl, EquipSlot ); // Remove it from the girl's inventory if they can't equip
            return false;
        }
    }
    else
        return false;
}

bool BrothelManager::AutomaticSlotlessItemUse( Girl* girl, int InvNum, std::string message )
{
    // Slotless items include manuals, stripper poles, free weights, etc...
    int EquipSlot = -1;
    
    EquipSlot = g_Girls.AddInv( girl, m_Inventory[InvNum] );
    
    if( EquipSlot != -1 )
    {
        RemoveItemFromInventoryByNumber( InvNum ); // Remove from general inventory
        g_InvManager.Equip( girl, EquipSlot, false );
        girl->m_Events.AddMessage( message, IMGTYPE_DEATH, EVENT_WARNING );
        return true;
    }
    else
        return false;
}

bool BrothelManager::AutomaticFoodItemUse( Girl* girl, int InvNum, std::string message )
{
    int EquipSlot = -1;
    
    EquipSlot = g_Girls.AddInv( girl, m_Inventory[InvNum] );
    
    if( EquipSlot != -1 )
    {
        RemoveItemFromInventoryByNumber( InvNum );
        g_InvManager.Equip( girl, EquipSlot, true );
        girl->m_Events.AddMessage( message, IMGTYPE_DEATH, EVENT_WARNING );
        return true;
    }
    else
        return false;
}

void BrothelManager::UpdateBribeInfluence()
{
    m_Influence = GetBribeRate();
    cRival* rival = GetRivals();
    
    if( rival )
    {
        long total = m_BribeRate;
        total += TOWN_OFFICIALSWAGES;   // this is the amount the government controls
        
        while( rival )  // get the total for all bribes
        {
            total += rival->m_BribeRate;
            rival = rival->m_Next;
        }
        
        rival = GetRivals();
        
        while( rival )  // get the total for all bribes
        {
            if( rival->m_BribeRate > 0 && total != 0 )
                rival->m_Influence = static_cast<int>( ( static_cast<float>( rival->m_BribeRate ) / static_cast<float>( total ) ) * 100.0f );
            else
                rival->m_Influence = 0;
                
            rival = rival->m_Next;
        }
        
        if( m_BribeRate != 0 && total != 0 )
            m_Influence = static_cast<int>( ( static_cast<float>( m_BribeRate ) / static_cast<float>( total ) ) * 100.0f );
        else
            m_Influence = 0;
    }
    else
    {
        if( m_BribeRate <= 0 )
            m_Influence = 0;
        else
            m_Influence = static_cast<int>( ( static_cast<float>( m_BribeRate ) / static_cast<float>( static_cast<float>( TOWN_OFFICIALSWAGES ) + static_cast<float>( m_BribeRate ) ) ) * 100.0f );
    }
}

void BrothelManager::do_tax()
{
    cConfig cfg;
    double taxRate = cfg.tax.rate();    // normal tax rate is 6%
    
    if( m_Influence > 0 ) // can you influence it lower
    {
        int lowerBy = m_Influence / 20;
        float amount = static_cast<float>( lowerBy / 100 );
        taxRate -= amount;
        
        if( taxRate < cfg.tax.minimum() )
            taxRate = cfg.tax.minimum();
    }
    
    // check for money laundering and apply tax
    int earnings = g_Gold.total_earned();
    
    if( earnings <= 0 )
    {
        g_MessageQue.AddToQue(
            "You didn't earn any money so didn't get taxed.", 0
        );
        return;
    }
    
    /*
     *  money laundering: nice idea - I had no idea it was
     *  in the game.
     *
     *  Probably we should make the player work for this.
     *  invest a little in businesses to launder through.
     */
    int laundry = g_Dice.random( static_cast<int>( earnings * cfg.tax.laundry() ) );
    long tax = static_cast<long>( ( earnings - laundry ) * taxRate );
    
    /*
     *  this should not logically happen unless we
     *  do something very clever with the money laundering
     */
    if( tax <= 0 )
    {
        g_MessageQue.AddToQue(
            "Thanks to a clever accountant, "
            "none of your income turns out to be taxable", 0
        );
        return;
    }
    
    g_Gold.tax( tax );
    std::stringstream ss;
    /*
     *  Let's report the laundering, at least.
     *  Otherwise, it just makes the tax rate wobble a bit
     */
    ss << "You were taxed " << tax << " gold. "
       << "You managed to launder " << laundry
       << " through various local businesses."
       ;
    g_MessageQue.AddToQue( ss.str(), 0 );
}

void BrothelManager::do_food_and_digs( Brothel* brothel, Girl* girl )
{
    /*
     *  add the girls accomodation and food costs to the upkeep
     */
    if( girl->is_slave() )
    {
        /*
         *      For a slavegirl, 5 gold per accomodation level
         */
        brothel->m_Finance.girl_support( 5 * ( girl->m_AccLevel + 1 ) );
        
        /*
         *      accomodation zero is all they expect
         */
        if( girl->m_AccLevel == 0 )
        {
            return;
        }
        
        /*
         *      accomodation over 0 means happier,
         *      and maybe liking the PC more
         *
         *      mod: docclox - made happiness gains greater
         *      for nicer digs
         */
        girl->happiness( 4 + girl->m_AccLevel / 2 );
        // end mod
        /*
         *      mod - doc - make love and hate change faster
         *      with better digs - but not at the same rate as
         *      the happiness bonus, so there's a point to the
         *      intermediate levels
         */
        int excess = girl->happiness() - 100;
        
        if( excess >= 0 )
        {
            int mod = 1 + excess / 3;
            girl->pchate( -mod );
            girl->pclove( mod );
        }
        
        // end mod
        return;
    }
    
    /*
     *  For a slavegirl, 20 gold per accomodation level
     *  mod - doc - simplified the calculation a bit
     */
    brothel->m_Finance.girl_support( 20 * ( girl->m_AccLevel + 1 ) );
    
    /*
     *  let's do the simple case
     *  if her accomodation is greater then her level
     *  she'll get happier. That's a mod: it was >=
     *  before, but this way 0 level girls want level 1 accom
     *  and it goes up level for level thereafter
     */
    if( girl->m_AccLevel > girl->level() )
    {
        girl->happiness( 2 + girl->m_AccLevel / 2 );
        int excess = girl->happiness() - 100;
        
        if( excess >= 0 )
        {
            int mod = 1 + excess / 3;
            girl->pchate( -mod );
            girl->pclove( mod );
        }
        
        return;
    }
    
    /*
     *  If we get here, the accomodation level is less
     *  than a girl of her accomplisments would expect
     *  However, level 6 and greater and her sense of
     *  professionalism means she doesn't let it affect her
     *  state of mind
     */
    if( girl->level() >= 6 )
    {
        return;
    }
    
    /*
     *  Failing that, she will be less happy
     */
    int mod, diff =  girl->level() - girl->m_AccLevel;
    mod = diff / 2; // half the difference, round down
    mod ++;     // and add one
    girl->happiness( -mod );
    
    /*
     *  and if she gets completely miserable,
     *  she'll grow to hate the PC
     */
    if( girl->happiness() <= 0 )
    {
        girl->pchate( 1 + diff / 3 );
    }
}

void BrothelManager::SortInventory()
{
//  qu_sort(0,299,m_Inventory);
}

int BrothelManager::HasItem( std::string name, int countFrom )
{
    // We look for an item in the range of countFrom to MAXNUM_INVENTORY.
    // Either the index of the item or -1 is returned.
    
    if( countFrom == -1 )
        countFrom = 0;
        
    if( countFrom >= MAXNUM_INVENTORY )
        return -1;
        
    for( int i = countFrom; i < MAXNUM_INVENTORY; i++ )
    {
        if( m_Inventory[i] )
        {
            if( m_Inventory[i]->m_Name == name )
                return i;
        }
    }
    
    return -1;
}

bool BrothelManager::AddItemToInventory( sInventoryItem* item )
{
    bool added = false;
    int curI = g_Brothels.HasItem( item->m_Name, -1 );
    
    bool loop = true;
    
    while( loop )
    {
        if( curI != -1 )
        {
            if( g_Brothels.m_NumItem[curI] >= 254 )
                curI = g_Brothels.HasItem( item->m_Name, curI + 1 );
            else
                loop = false;
        }
        else
            loop = false;
    }
    
    if( g_Brothels.m_NumInventory < MAXNUM_INVENTORY || curI != -1 )
    {
        if( curI != -1 )
        {
            added = true;
            g_Brothels.m_NumItem[curI]++;
            
        }
        else
        {
            for( int j = 0; j < MAXNUM_INVENTORY; j++ )
            {
                if( g_Brothels.m_Inventory[j] == nullptr )
                {
                    added = true;
                    g_Brothels.m_Inventory[j] = item;
                    g_Brothels.m_EquipedItems[j] = 0;
                    g_Brothels.m_NumInventory++;
                    g_Brothels.m_NumItem[j]++;
                    break;
                }
            }
        }
    }
    
    SortInventory();
    return added;
}

bool BrothelManager::RemoveItemFromInventoryByNumber( int Pos )
{
    bool removed = false;
    
    if( g_Brothels.m_Inventory[Pos] != nullptr )
    {
        if( g_Brothels.m_NumItem[Pos] > 0 )
        {
            removed = true;
            g_Brothels.m_NumItem[Pos]--;
            
            // We may reduce the stack size to zero
            if( g_Brothels.m_NumItem[Pos] == 0 )
            {
                g_Brothels.m_Inventory[Pos] = nullptr;
                g_Brothels.m_EquipedItems[Pos] = 0;
                g_Brothels.m_NumInventory--;
            }
        } // if num > 0
    }      // Inventory type not null
    
    SortInventory();
    return removed;
}

void BrothelManager::WithdrawFromBank( long amount )
{
    if( m_Bank - amount >= 0 )
        m_Bank -= amount;
}

void BrothelManager::DepositInBank( long amount )
{
    if( amount > 0 )
        m_Bank += amount;
}

double BrothelManager::calc_pilfering( Girl* girl )
{
    double factor = 0.0;
    
    /*
     *  on top of all other factors,
     *  an addict will steal to feed her habit
     */
    if( girl->is_addict() && girl->m_Money == 0 )
    {
        factor += 0.2;
    }
    
    /*
     *  let's work out what if she is going steal anything
     *
     *  love or obedience will heep her honest
     */
    if( girl->pclove() >= 50 || girl->obedience() >= 50 )
    {
        return factor;
    }
    
    /*
     *  if her fear is greater than her hate, she won't dare steal
     */
    if( girl->pcfear() > girl->pchate() )
    {
        return factor;
    }
    
    /*
     *  and apparently, slaves don't steal
     */
    if( girl->is_slave() )
    {
        return factor;
    }
    
    /*
     *  given all the above, if she hates him enough, she'll steal
     */
    if( girl->pchate() > 40 )
    {
        return factor + 0.15;
    }
    
    /*
     *  if she's not motivated by hatred, she needs to be pretty confident
     *
     */
    if( girl->confidence() > 70 && girl->spirit() > 50 )
    {
        return factor + 0.15;
    }
    
    /*
     *  otherwise, she stays honest
     */
    return 0.0;
}

void BrothelManager::CalculatePay( Brothel* brothel, Girl* girl, u_int Job )
{
    if(
        girl->m_Stats[STAT_HOUSE] == 0
//      || girl->m_DayJob == JOB_MATRON  // Matrons get full pay
        || m_JobManager.is_job_Paid_Player( Job )               // WD:  added test for paid jobs
    )
    {
        girl->m_Money += girl->m_Pay;
        girl->m_Pay = 0;
        return;
    }
    
    if( girl->m_Pay <= 0 )
    {
        if( girl->m_Pay < 0 )
            girl->m_Pay = 0;
            
        return;
    }
    
    double steal_factor = calc_pilfering( girl );
    float house_factor = float( girl->m_Stats[STAT_HOUSE] ) / 100.0f;
    /*
     *  work out how much gold (if any) she steals
     */
    int stolen = int( steal_factor * girl->m_Pay );
    girl->m_Pay -= stolen;
    /*
     *  the house takes its cut of whatever's left
     */
    int house = int( house_factor * girl->m_Pay );
    /*
     *  add the various portions of the take to the
     *  girl and the brothel finances
     */
    brothel->m_Finance.brothel_work( house );
    girl->m_Money += girl->m_Pay - house;
    girl->m_Money += stolen;    // she secrets away some money
    
    /*
     *  this shouldn't happen. That said...
     */
    if( house > girl->m_Pay ) house = girl->m_Pay;
    
    /*
     *  Not sure how this could happen - suspect it's just
     *  a sanity check
     */
    if( girl->m_Money < 0 ) girl->m_Money = 0;
    
    /*
     *  If she didn't steal anything, we're done
     */
    if( !stolen )
        return;
        
    /*
     *  if no-one is watching for theft, we're done
     */
    sGang* gang = g_Gangs.GetGangOnMission( MISS_SPYGIRLS );
    
    if( !gang )
        return;
        
    /*
     *  work out the % chance that the girl gets caught
     */
    int catch_pc = g_Gangs.chance_to_catch( girl );
    
    /*
     *  if they don't catch her, we're done
     */
    if( !g_Dice.percent( catch_pc ) )
        return;
        
    /*
     *  OK: she got caught. Tell the player
     */
    std::string gmess = "Your Goons spotted ";
    gmess += girl->m_Realname;
    gmess += " taking more gold then she reported.";
    gang->m_Events.AddMessage( gmess, IMGTYPE_PROFILE,  EVENT_GANG );
    
    girl->m_Pay = 0;
}

void BrothelManager::UpdateObjective()
{
    if( m_Objective )
    {
        if( m_Objective->m_Limit != -1 )
            m_Objective->m_Limit--;
            
        if( m_Objective->m_Limit == 0 )
        {
            g_MessageQue.AddToQue( "You have failed an objective.", 2 );
            delete m_Objective;
            m_Objective = nullptr;
            return;
        }
        
        switch( m_Objective->m_Objective )
        {
        case OBJECTIVE_REACHGOLDTARGET:
            if( g_Gold.ival() >= m_Objective->m_Target )
                PassObjective();
                
            break;
            
        case OBJECTIVE_HAVEXGOONS:
            if( g_Gangs.GetNumGangs() >= m_Objective->m_Target )
                PassObjective();
                
            break;
            
        case OBJECTIVE_STEALXAMOUNTOFGOLD:
        case OBJECTIVE_CAPTUREXMONSTERGIRLS:
        case OBJECTIVE_KIDNAPXGIRLS:
        case OBJECTIVE_EXTORTXNEWBUSSINESS:
            if( m_Objective->m_SoFar >= m_Objective->m_Target )
                PassObjective();
                
            break;
            
        case OBJECTIVE_HAVEXMONSTERGIRLS:
            if( GetTotalNumGirls( true ) >= m_Objective->m_Target )
                PassObjective();
                
            break;
            
        case OBJECTIVE_HAVEXAMOUNTOFGIRLS:
            if( GetTotalNumGirls() >= m_Objective->m_Target )
                PassObjective();
                
            break;
            
        default:
            g_LogFile.ss() << "switch (m_Objective->m_Objective): unknown value \"" << m_Objective->m_Objective << "\"\n" << __FILE__ << " " << __LINE__ << "\n";
        }
    }
}

Objective* BrothelManager::GetObjective()
{
    return m_Objective;
}

void BrothelManager::CreateNewObjective()
{
    if( m_Objective )
        delete m_Objective;
        
    m_Objective = nullptr;
    
    m_Objective = new Objective();
    
    if( m_Objective )
    {
        std::string message = "You have a new objective, you must ";
        bool done = false;
        m_Objective->m_Difficulty = g_Year - 1209;
        m_Objective->m_SoFar = 0;
        m_Objective->m_Reward = g_Dice % NUM_REWARDS;
        m_Objective->m_Limit = -1;
        m_Objective->m_Target = 0;
        
        while( !done )
        {
            m_Objective->m_Objective = g_Dice % NUM_OBJECTIVES;
            
            switch( m_Objective->m_Objective )
            {
            case OBJECTIVE_REACHGOLDTARGET:
                message += "Acquire ";
                
                if( m_Objective->m_Difficulty >= 3 )
                {
                    m_Objective->m_Limit = ( g_Dice % 20 ) + 10;
                    m_Objective->m_Target = m_Objective->m_Limit * 1000;
                    message += toString( m_Objective->m_Target );
                    message += " gold within ";
                    message += toString( m_Objective->m_Limit );
                    message += " weeks.";
                }
                else
                {
                    m_Objective->m_Target = ( ( g_Dice % 20 ) + 1 ) * 200;
                    message += toString( m_Objective->m_Target );
                    message += " gold.";
                }
                
                done = true;
                break;
                
            case OBJECTIVE_LAUNCHSUCCESSFULATTACK:
                message += "Launch a successful attack mission within ";
                
                if( m_Objective->m_Difficulty >= 3 )
                    m_Objective->m_Limit = ( g_Dice % 5 ) + 3;
                else
                    m_Objective->m_Limit = ( g_Dice % 10 ) + 10;
                    
                message += toString( m_Objective->m_Limit );
                message += " weeks.";
                done = true;
                break;
                
            case OBJECTIVE_HAVEXGOONS:
                if( g_Gangs.GetNumGangs() < 8 )
                {
                    message += "Have ";
                    m_Objective->m_Target = g_Gangs.GetNumGangs() + ( ( g_Dice % 3 ) + 1 );
                    
                    if( m_Objective->m_Target > 8 )
                        m_Objective->m_Target = 8;
                        
                    message += toString( m_Objective->m_Target );
                    message += " gangs within ";
                    
                    if( m_Objective->m_Difficulty >= 3 )
                        m_Objective->m_Limit = ( g_Dice % 4 ) + 3;
                    else
                        m_Objective->m_Limit = ( g_Dice % 7 ) + 6;
                        
                    message += toString( m_Objective->m_Limit );
                    message += " weeks.";
                    done = true;
                }
                
                break;
                
            case OBJECTIVE_STEALXAMOUNTOFGOLD:
                if( m_Objective->m_Difficulty >= 2 )
                {
                    message += "Steal ";
                    m_Objective->m_Limit = ( g_Dice % 20 ) + 13;
                    m_Objective->m_Target = m_Objective->m_Limit * 1300;
                    message += toString( m_Objective->m_Target );
                    message += " gold within ";
                    message += toString( m_Objective->m_Limit );
                    message += " weeks.";
                }
                else
                {
                    m_Objective->m_Target = ( ( g_Dice % 20 ) + 1 ) * 200;
                    message += "Steal ";
                    message += toString( m_Objective->m_Target );
                    message += " gold.";
                }
                
                done = true;
                break;
                
            case OBJECTIVE_CAPTUREXMONSTERGIRLS:
                message += "Capture ";
                
                if( m_Objective->m_Difficulty >= 2 )
                {
                    m_Objective->m_Limit = ( g_Dice % 5 ) + 1;
                    m_Objective->m_Target = ( g_Dice % ( m_Objective->m_Limit - 1 ) ) + 1;
                    message += toString( m_Objective->m_Target );
                    message += " monster girls from the catacombs within ";
                    message += toString( m_Objective->m_Limit );
                    message += " weeks.";
                }
                else
                {
                    m_Objective->m_Target = ( g_Dice % 5 ) + 1;
                    message += toString( m_Objective->m_Target );
                    message += " monster girls from the catacombs.";
                }
                
                done = true;
                break;
                
            case OBJECTIVE_HAVEXMONSTERGIRLS:
                message += "Have a total of ";
                
                if( m_Objective->m_Difficulty >= 2 )
                {
                    m_Objective->m_Limit = ( g_Dice % 8 ) + 3;
                    m_Objective->m_Target = GetTotalNumGirls( true ) + ( g_Dice % ( m_Objective->m_Limit - 1 ) ) + 1;
                    message += toString( m_Objective->m_Target );
                    message += " monster girls within ";
                    message += toString( m_Objective->m_Limit );
                    message += " weeks.";
                }
                else
                {
                    m_Objective->m_Target = GetTotalNumGirls( true ) + ( g_Dice % 8 ) + 1;
                    message += toString( m_Objective->m_Target );
                    message += " monster girls.";
                }
                
                done = true;
                break;
                
            case OBJECTIVE_KIDNAPXGIRLS:
                {
                    message += "Kidnap ";
                    
                    if( m_Objective->m_Difficulty >= 2 )
                    {
                        m_Objective->m_Limit = ( g_Dice % 5 ) + 1;
                        m_Objective->m_Target = ( g_Dice % ( m_Objective->m_Limit - 1 ) ) + 1;
                        message += toString( m_Objective->m_Target );
                        message += " girls from the streets within ";
                        message += toString( m_Objective->m_Limit );
                        message += " weeks.";
                    }
                    else
                    {
                        m_Objective->m_Target = ( g_Dice % 5 ) + 1;
                        message += toString( m_Objective->m_Target );
                        message += " girls from the streets.";
                    }
                    
                    done = true;
                }
                break;
                
            case OBJECTIVE_EXTORTXNEWBUSSINESS:
                {
                    message += "Gain control of ";
                    
                    if( m_Objective->m_Difficulty >= 2 )
                    {
                        m_Objective->m_Limit = ( g_Dice % 5 ) + 1;
                        m_Objective->m_Target = ( g_Dice % ( m_Objective->m_Limit - 1 ) ) + 1;
                        message += toString( m_Objective->m_Target );
                        message += " new bussinesses within ";
                        message += toString( m_Objective->m_Limit );
                        message += " weeks.";
                    }
                    else
                    {
                        m_Objective->m_Target = ( g_Dice % 5 ) + 1;
                        message += toString( m_Objective->m_Target );
                        message += " new bussinesses.";
                    }
                    
                    done = true;
                }
                break;
                
            case OBJECTIVE_HAVEXAMOUNTOFGIRLS:
                message += "Have a total of ";
                
                if( m_Objective->m_Difficulty >= 2 )
                {
                    m_Objective->m_Limit = ( g_Dice % 8 ) + 3;
                    m_Objective->m_Target = GetTotalNumGirls() + ( g_Dice % ( m_Objective->m_Limit - 1 ) ) + 1;
                    message += toString( m_Objective->m_Target );
                    message += " girls within ";
                    message += toString( m_Objective->m_Limit );
                    message += " weeks.";
                }
                else
                {
                    m_Objective->m_Target = GetTotalNumGirls() + ( g_Dice % 8 ) + 1;
                    message += toString( m_Objective->m_Target );
                    message += " girls.";
                }
                
                done = true;
                break;
                
            case OBJECTIVE_GETNEXTBROTHEL:
                if( GetNumBrothels() < 6 )
                {
                    message += "Purchase a new brothel";
                    
                    if( m_Objective->m_Difficulty >= 2 )
                    {
                        m_Objective->m_Limit = ( g_Dice % 10 ) + 10;
                        message += " within ";
                        message += toString( m_Objective->m_Limit );
                        message += " weeks.";
                    }
                    else
                        message += ".";
                        
                    done = true;
                }
                
                break;
                
            default:
                g_LogFile.ss() << "switch (m_Objective->m_Objective): unknown value \"" << m_Objective->m_Objective << "\"\n" << __FILE__ << " " << __LINE__ << "\n";
            }
        }
        
        g_MessageQue.AddToQue( message, 2 );
    }
}

void BrothelManager::PassObjective()
{
    if( m_Objective )
    {
        std::string message = "You have completed your objective and you get ";
        
        switch( m_Objective->m_Reward )
        {
        case REWARD_GOLD:
            {
                long gold = ( g_Dice % 200 ) + 33;
                
                if( m_Objective->m_Difficulty > 0 )
                    gold *= m_Objective->m_Difficulty;
                    
                message += toString( gold );
                message += " gold.";
                g_Gold.objective_reward( gold );
            }
            break;
            
        case REWARD_GIRLS:
            {
                int girls = 1;
                
                if( m_Objective->m_Difficulty > 0 )
                    girls *= m_Objective->m_Difficulty;
                    
                message += toString( girls );
                message += " slave girls.";
                
                while( girls > 0 )
                {
                    m_Dungeon.AddGirl( g_Girls.CreateRandomGirl( 0, true, "", true, true ), DUNGEON_NEWGIRL );
                    girls--;
                }
            }
            break;
            
        case REWARD_RIVALHINDER:
            {
                cRival* rival = m_Rivals.GetRandomRival();
                
                if( rival )
                {
                    long gold = 0;
                    message += "to steal ";
                    
                    if( rival->m_Gold > 10 )
                        gold = ( g_Dice % ( rival->m_Gold / 2 ) ) + 1;
                    else
                        gold = 436;
                        
                    rival->m_Gold -= gold;
                    message += toString( gold );
                    message += " gold from the ";
                    message += rival->m_Name;
                    message += ".";
                    g_Gold.objective_reward( gold );
                }
                else
                {
                    message = "";
                    m_Objective->m_Reward  = REWARD_GOLD;
                    PassObjective();
                }
            }
            break;
            
        case REWARD_ITEM:
            {
                int numItems = 1;
                
                if( m_Objective->m_Difficulty > 0 )
                    numItems *= m_Objective->m_Difficulty;
                    
                while( numItems > 0 )
                {
                    //mod
                    //purpose fix a crash
                    sInventoryItem* item = nullptr;
                    
                    do
                    {
                        item = g_InvManager.GetRandomItem();
                    }
                    while( !item );
                    
                    // end mod
                    if( item->m_Rarity != 5 )
                    {
                        int curI = g_Brothels.HasItem( item->m_Name, -1 );
                        bool loop = true;
                        
                        while( loop )
                        {
                            if( curI != -1 )
                            {
                                if( g_Brothels.m_NumItem[curI] >= 254 )
                                    curI = g_Brothels.HasItem( item->m_Name, curI + 1 );
                                else
                                    loop = false;
                            }
                            else
                                loop = false;
                        }
                        
                        if( g_Brothels.m_NumInventory < MAXNUM_INVENTORY || curI != -1 )
                        {
                            if( curI != -1 )
                            {
                                message += "a ";
                                message += item->m_Name;
                                message += ", ";
                                g_Brothels.m_NumItem[curI]++;
                            }
                            else
                            {
                                for( int j = 0; j < MAXNUM_INVENTORY; j++ )
                                {
                                    if( g_Brothels.m_Inventory[j] == nullptr )
                                    {
                                        message += "a ";
                                        message += item->m_Name;
                                        message += ", ";
                                        g_Brothels.m_Inventory[j] = item;
                                        g_Brothels.m_EquipedItems[j] = 0;
                                        g_Brothels.m_NumInventory++;
                                        g_Brothels.m_NumItem[j]++;
                                        break;
                                    }
                                }
                            }
                            
                            numItems--;
                        }
                        else
                        {
                            numItems = 0;
                            message += " Your inventory is full\n";
                        }
                    }
                }
            }
            break;
            
        default:
            g_LogFile.ss() << "switch (m_Objective->m_Reward): unknown value \"" << m_Objective->m_Reward << "\"\n" << __FILE__ << " " << __LINE__ << "\n";
        }
        
        if( message != "" )
            g_MessageQue.AddToQue( message, 2 );
            
        delete m_Objective;
        m_Objective = nullptr;
    }
}

void BrothelManager::AddCustomObjective( int limit, int diff, int objective, int reward, int sofar, int target )
{
    if( m_Objective )
        delete m_Objective;
        
    m_Objective = nullptr;
    
    m_Objective = new Objective();
    
    m_Objective->m_Difficulty = diff;
    m_Objective->m_Limit = limit;
    m_Objective->m_Objective = objective;
    m_Objective->m_Reward = reward;
    m_Objective->m_SoFar = sofar;
    m_Objective->m_Target = target;
}

int& BrothelManager::stat_lookup( std::string stat_name, int brothel_id )
{
    if( stat_name == "filth" )
        return this->GetBrothel( brothel_id )->m_Filthiness;
    else if( stat_name == "advertising" )
        return ( int& )GetBrothel( brothel_id )->m_AdvertisingBudget;
    else if( stat_name == "security" )
        return GetBrothel( brothel_id )->m_SecurityLevel;
    else if( stat_name == "beasts" )
        return this->m_Beasts;
        
    return m_Dummy;
}

void BrothelManager::SetGirlStat( Girl* girl, int stat, int amount )
{
    girl->m_StatMods[stat] = amount;
}

void BrothelManager::updateGirlTurnBrothelStats( Girl* girl )
{
    /*
     *  WD: Update each turn the stats for girl in brothel
     *
     *  Uses scaling formula in the form of
     *      bonus = (60  � STAT_HOUSE) / div
     *
     *              div =
     *      STAT    30  20  15
     *      0       2   3   4
     *      1       1   2   3
     *      10      1   2   3
     *      11      1   2   3
     *      20      1   2   2
     *      21      1   1   2
     *      30      1   1   2
     *      31      0   1   1
     *      40      0   1   1
     *      41      0   0   1
     *      50      0   0   0
     *      51      0   0   0
     *      60      0   0   0
     *      61      -1  -1  -1
     *      70      -1  -1  -1
     *      71      -1  -1  -1
     *      80      -1  -1  -2
     *      81      -1  -2  -2
     *      90      -1  -2  -2
     *      91      -2  -2  -3
     *      100     -2  -2  -3
     *
     *
     */
    
//#define WDTEST // debuging
#undef WDTEST

    std::string girlName    = girl->m_Realname;
    int statHouse   = girl->house();
    int bonus       = ( 60 - statHouse ) / 30;
    
    // Sanity check. Abort on dead girl
    if( girl->health() <= 0 )
    {
        return;
    }
    
    
#ifdef WDTEST // debuging
    
    std::string sum = "Start\n";
    sum += "   h=";
    sum += toString( girl->happiness() );
    sum += "   o=";
    sum += toString( girl->obedience() );
    sum += "   l=";
    sum += toString( girl->pclove() );
    sum += "   f=";
    sum += toString( girl->pcfear() );
    sum += "   h=";
    sum += toString( girl->pchate() );
    
    
    
    sum += "   HP=";
    sum += toString( girl->health() );
    sum += "  TD=";
    sum += toString( girl->tiredness() );
    
#endif
    
    if( girl->is_slave() )
    {
        if( bonus > 0 )                     // Slaves don't get penalties
        {
            girl->obedience( bonus );       // bonus vs house stat  0: 31-60, 1: 01-30, 2: 00
            girl->pcfear( -bonus );
            girl->pchate( -bonus );
            
            bonus = ( 60 - statHouse ) / 15;
            girl->happiness( bonus );       // bonus vs house stat  0: 46-60, 1: 31-45, 2: 16-30, 3: 01-15, 4: 00
        }
    }
    else                                    // Free girls
    {
        girl->obedience( bonus );           // bonus vs house stat  -2: 91-100, -1: 61-90, 0: 31-60, 1: 01-30, 2: 00
        
        if( bonus > 0 )                     // no increase for hate or fear
        {
            girl->pcfear( -bonus );
            girl->pchate( -bonus );
        }
        
        bonus = ( 60 - statHouse ) / 15;
        girl->happiness( bonus );           // bonus vs house stat  -3: 91-100, -2: 76-90, -1: 61-75, 0: 46-60, 1: 31-45, 2: 16-30, 3: 01-15, 4: 00
    }
    
#ifdef WDTEST // debuging
    
    sum += "\n\nFinal\n";
    sum += "   h=";
    sum += toString( girl->happiness() );
    sum += "   o=";
    sum += toString( girl->obedience() );
    sum += "   l=";
    sum += toString( girl->pclove() );
    sum += "   f=";
    sum += toString( girl->pcfear() );
    sum += "   h=";
    sum += toString( girl->pchate() );
    sum += "   HP=";
    sum += toString( girl->health() );
    sum += "  TD=";
    sum += toString( girl->tiredness() );
    
    girl->m_Events.AddMessage( sum, IMGTYPE_PROFILE, EVENT_DEBUG );
    
#undef WDTEST
#endif
}

void BrothelManager::UpdateAllGirlsStat( Brothel* brothel, int stat, int amount )
{
    if( brothel )
    {
        Girl* current = brothel->m_Girls;
        
        while( current )
        {
            g_Girls.UpdateStat( current, stat, amount );
            current = current->m_Next;
        }
    }
    else
    {
        Brothel* curBroth = m_Parent;
        
        while( curBroth )
        {
            Girl* current = curBroth->m_Girls;
            
            while( current )
            {
                g_Girls.UpdateStat( current, stat, amount );
                current = current->m_Next;
            }
            
            curBroth = curBroth->m_Next;
        }
    }
}

Girl* BrothelManager::GetGirl( int brothelID, int num )
{
    if( num < 0 )
        return nullptr;
        
    Brothel* current = m_Parent;
    
    while( current )
    {
        if( current->m_id == brothelID )
            break;
            
        current = current->m_Next;
    }
    
    if( num > current->m_NumGirls )
        return nullptr;
        
    if( current )
    {
        int count = 0;
        Girl* currentGirl = current->m_Girls;
        
        while( currentGirl )
        {
            if( count == num )
                break;
                
            count++;
            currentGirl = currentGirl->m_Next;
        }
        
        return currentGirl;
    }
    
    return nullptr;
}

int BrothelManager::GetGirlPos( int brothelID, Girl* girl )
{
    Brothel* current = m_Parent;
    
    while( current )
    {
        if( current->m_id == brothelID )
            break;
            
        current = current->m_Next;
    }
    
    if( current )
    {
        int count = 0;
        Girl* currentGirl = current->m_Girls;
        
        while( currentGirl )
        {
            if( currentGirl == girl )
                return count;
                
            count++;
            currentGirl = currentGirl->m_Next;
        }
    }
    
    return -1;
}

Girl* BrothelManager::GetGirlByName( int brothelID, std::string name )
{
    // Get the proper brothel
    Brothel* current = m_Parent;
    
    while( current )
    {
        if( current->m_id == brothelID )
            break;
            
        current = current->m_Next;
    }
    
    // Find the girl
    if( current )
    {
        Girl* currentGirl = current->m_Girls;
        
        while( currentGirl )
        {
            if( strnatcmp( name.c_str(), currentGirl->m_Realname.c_str() ) == 0 )
                break;
                
            currentGirl = currentGirl->m_Next;
        }
        
        return currentGirl;
    }
    
    return nullptr;
}

int BrothelManager::GetGirlsCurrentBrothel( Girl* girl )
{
    // Used by new security guard code
    int BrothelNo = -1;
    
    for( int i = 0; i < m_NumBrothels; i++ )
    {
        BrothelNo = GetGirlPos( i, girl );
        
        // MYR: Im an idiot. I wrote /= for 'not equals' (which is B syntax)
        // instead of !=
        if( BrothelNo != -1 )
            return i;
    }
    
    return -1;
}

std::vector<Girl*> BrothelManager::GirlsOnJob( int BrothelID, int JobID, bool day )
{
    // Used by new security code
    Brothel* current = m_Parent;
    
    while( current )
    {
        if( current->m_id == BrothelID )
            break;
            
        current = current->m_Next;
    }
    
    std::vector<Girl*> GirlsOnJob;
    
    Girl* curr = current->m_Girls;
    
    while( curr )
    {
        if( day )
        {
            if( curr->m_DayJob == JobID )
                GirlsOnJob.push_back( curr );
        }
        else
        {
            if( curr->m_NightJob == JobID )
                GirlsOnJob.push_back( curr );
        }
        
        curr = curr->m_Next;
    }
    
    return GirlsOnJob;
}

int BrothelManager::GetTotalNumGirls( bool monster )
{
    int total = 0;
    
    if( !monster )
    {
        for( int i = 0; i < m_NumBrothels; i++ )
            total += GetNumGirls( i );
            
        total += GetDungeon()->GetNumGirls();
    }
    else
    {
        Brothel* current = m_Parent;
        
        while( current )
        {
            Girl* girl = current->m_Girls;
            
            while( girl )
            {
                if( g_Girls.HasTrait( girl, "Not Human" ) )
                    total++;
                    
                girl = girl->m_Next;
            }
            
            current = current->m_Next;
        }
        
        for( int i = 0; i < GetDungeon()->GetNumGirls(); i++ )
        {
            sDungeonGirl* dgirl = GetDungeon()->GetGirl( i );
            
            if( g_Girls.HasTrait( dgirl->m_Girl, "Not Human" ) )
                total++;
        }
    }
    
    return total;
}

std::string BrothelManager::GetGirlString( int brothelID, int girlNum )
{
    std::string data = "";
    Brothel* current = m_Parent;
    
    while( current )
    {
        if( current->m_id == brothelID )
            break;
            
        current = current->m_Next;
    }
    
    if( current )
    {
        int count = 0;
        Girl* currentGirl = current->m_Girls;
        
        while( currentGirl )
        {
            if( count == girlNum )
                break;
                
            count++;
            currentGirl = currentGirl->m_Next;
        }
        
//need to change into string lookup
        std::string dayjob = "Resting";
        
        switch( static_cast<int>( currentGirl->m_DayJob ) )
        {
        case JOB_WHORESTREETS:
            dayjob = "Streets";
            break;
            
        case JOB_WHOREBROTHEL:
            dayjob = "Indoors";
            break;
            
        case JOB_INDUNGEON:
            dayjob = "Dungeon";
            break;
            
        case JOB_TRAINING:
            dayjob = "Training";
            break;
            
        default:
            std::stringstream local_ss;
            local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
            g_LogFile.write( local_ss.str() );
            break;
        }
        
        std::string nightjob = "Resting";
        
        switch( static_cast<int>( currentGirl->m_NightJob ) )
        {
        case JOB_WHORESTREETS:
            nightjob = "Streets";
            break;
            
        case JOB_WHOREBROTHEL:
            nightjob = "Indoors";
            break;
            
        case JOB_INDUNGEON:
            nightjob = "Dungeon";
            break;
            
        case JOB_TRAINING:
            nightjob = "Training";
            break;
            
        default:
            std::stringstream local_ss;
            local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
            g_LogFile.write( local_ss.str() );
            break;
        }
        
        if( g_Girls.GetStat( currentGirl, STAT_AGE ) == 100 )
            sprintf( buffer, "%s | Age: unknown | Health: %i | Happiness %i | Day Job: %s | Night Job: %s", currentGirl->m_Realname.c_str(), static_cast<int>( g_Girls.GetStat( currentGirl, STAT_HEALTH ) ), static_cast<int>( g_Girls.GetStat( currentGirl, STAT_HAPPINESS ) ), dayjob.c_str(), nightjob.c_str() );
        else
            sprintf( buffer, "%s | Age: %i | Health: %i | Happiness %i | Day Job: %s | Night Job: %s", currentGirl->m_Realname.c_str(), g_Girls.GetStat( currentGirl, STAT_AGE ), static_cast<int>( g_Girls.GetStat( currentGirl, STAT_HEALTH ) ), static_cast<int>( g_Girls.GetStat( currentGirl, STAT_HAPPINESS ) ), dayjob.c_str(), nightjob.c_str() );
            
        data = buffer;
    }
    
    return data;
}

std::string BrothelManager::GetName( int brothelID )
{
    Brothel* current = m_Parent;
    
    while( current )
    {
        if( current->m_id == brothelID )
            break;
            
        current = current->m_Next;
    }
    
    if( current )
        return current->m_Name;
        
    return "BrothelManager::GetName - Something went wrong";
}

std::string BrothelManager::GetBrothelString( int brothelID )
{
    std::stringstream ss;
    Brothel* brothel = GetBrothel( brothelID );
    
    /*
     *  if we can't find the brothel, go home
     *  the error is logged in GetBrothel,
     *  so just return an empty string
     */
    if( !brothel )
    {
        return "";
    }
    
    /*
     *  some shorthand variables for the simpler descriptions
     *  Commented out since we may want to put these back in at some stage
     *
        const char *has_bar =  (
            brothel->m_Bar == 1 ? "Yes" : "No"
        );
        const char *has_hall =  (
            brothel->m_GamblingHall == 1 ? "Yes" : "No"
        );
     */
    long profit = brothel->m_Finance.total_profit();
    /*
     *  format the summary into one big string, and return it
     */
    ss << "Customer Happiness: "    << happiness_text( brothel ) << std::endl;
    ss << "Fame: "          << fame_text( brothel );
    ss << "Rooms (available/total): "
       << int( brothel->m_NumRooms ) - int( brothel->m_NumGirls )
       << "/"
       << int( brothel->m_NumRooms )
       << std::endl;
    //ss << "Strip Bar: "       << has_bar      << std::endl;
    //ss << "Gambling Hall: "   << has_hall     << std::endl;
    ss << "This brothel's Profit: " << profit       << std::endl;
    ss << "Your Gold: "     << g_Gold.ival()    << std::endl;
    //ss << "Gambling Pool: "       << m_GamblingHallPool   << std::endl;
    ss << "Disposition: "       << disposition_text()   << std::endl;
    ss << "Suspicion: "     << suss_text()      << std::endl    ;
    ss << "Filthiness: " <<  brothel->m_Filthiness << std::endl;
    ss << "Beasts Owned: " <<  g_Brothels.GetNumBeasts() << std::endl;
    return ss.str();
//add cleanliness and check gh and bh
}

Brothel* BrothelManager::GetBrothel( int brothelID )
{
    Brothel* current = m_Parent;
    
    while( current )
    {
        if( current->m_id == brothelID )
            return current;
            
        current = current->m_Next;
    }
    
    /*
     *  brothel not found at least deserves a log message
     */
    std::stringstream ss;
    ss << "Brothel " << brothelID << " not found in list!";
    g_LogFile.write( ss.str() );
    return nullptr;
}

int BrothelManager::GetNumBrothelsWithVacancies()
{
    int number = 0;
    Brothel* current = m_Parent;
    
    while( current )
    {
        if( current->m_NumGirls < current->m_NumRooms )
            number++;
            
        current = current->m_Next;
    }
    
    return number;
}

int BrothelManager::GetNumGirls( int brothelID )
{
    Brothel* current = m_Parent;
    
    while( current )
    {
        if( current->m_id == brothelID )
            break;
            
        current = current->m_Next;
    }
    
    if( current )
        return current->m_NumGirls;
        
    return 0;
}

int BrothelManager::GetNumGirlsOnJob( int brothelID, int jobID, bool day )
{
    int count = 0;
    Brothel* current = m_Parent;
    
    if( brothelID != -1 )
    {
        while( current )
        {
            if( current->m_id == brothelID )
                break;
                
            current = current->m_Next;
        }
    }
    
//      if(g_Brothels.GetNumGirlsOnJob(-1, JOB_TORTURER, DayOrNight) == 1)

    while( current )
    {
        Girl* curr = current->m_Girls;
        
        while( curr )
        {
            if( day )
            {
                if( curr->m_DayJob == jobID )
                    count++;
            }
            else
            {
                if( curr->m_NightJob == jobID )
                    count++;
            }
            
            curr = curr->m_Next;
        }
        
        if( brothelID == -1 )
            current = current->m_Next;
        else
            current = nullptr;
    }
    
    return count;
}

void BrothelManager::SetName( int brothelID, std::string name )
{
    std::string data = "";
    Brothel* current = m_Parent;
    
    while( current )
    {
        if( current->m_id == brothelID )
            break;
            
        current = current->m_Next;
    }
    
    if( current )
        current->m_Name = name;
    else
        current->m_Name =  "BrothelManager::GetName - Something went wrong";
}

std::string BrothelManager::disposition_text()
{
    if( m_Player.disposition() >= 80 )    return "Benevolent";
    
    if( m_Player.disposition() >= 50 )    return "Nice";
    
    if( m_Player.disposition() > 10 )     return "Pleasant";
    
    if( m_Player.disposition() >= -10 )   return "Neutral";
    
    if( m_Player.disposition() > -50 )    return "Not nice";
    
    if( m_Player.disposition() > -80 )    return "Mean";
    
    return "Evil";
}

std::string BrothelManager::fame_text( Brothel* brothel )
{
    if( brothel->m_Fame >= 90 )   return  "World Renowned\n";
    
    if( brothel->m_Fame >= 80 )   return  "Famous\n";
    
    if( brothel->m_Fame >= 70 )   return  "Well Known\n";
    
    if( brothel->m_Fame >= 60 )   return  "Talk of the town\n";
    
    if( brothel->m_Fame >= 50 )   return  "Somewhat known\n";
    
    if( brothel->m_Fame >= 30 )   return  "Mostly unknown\n";
    
    return  "Unknown\n";
}

std::string BrothelManager::suss_text()
{
    //WD:   Should be Susipicion not Disposition
    if( m_Player.suspicion() >= 80 )  return "Town Scum";
    
    if( m_Player.suspicion() >= 50 )  return "Miscreant";
    
    if( m_Player.suspicion() >= 10 )  return "Suspect";
    
    if( m_Player.suspicion() >= -10 ) return "Unsuspected";
    
    if( m_Player.suspicion() >= -50 ) return "Lawful";
    
    if( m_Player.suspicion() >= -80 )     return "Philanthropist";
    
    return "Town Hero";
    
    //if(m_Player.m_Disposition >= 80)  return "Town Scum";
    //if(m_Player.m_Disposition >= 50)  return "Miscreant";
    //if(m_Player.m_Disposition >= 10)      return "Suspect";
    //if(m_Player.m_Disposition >= -10) return "Unsuspected";
    //if(m_Player.m_Disposition >= -50) return "Lawful";
    //if(m_Player.m_Disposition >= -80)     return "Philanthropist";
    //                  return "Town Hero";
}

std::string BrothelManager::happiness_text( Brothel* brothel )
{
    if( brothel->m_Happiness >= 80 )      return "High";
    
    if( brothel->m_Happiness < 40 )       return "Low";
    else                    return "Medium";
}

// True means the girl beat the brothel master
bool BrothelManager::PlayerCombat( Girl* girl ) // **************************** for now doesn't count items
{
    // MYR: Sanity check: Incorporial is an auto-win.
    if( girl->has_trait( "Incorporial" ) )
    {
        girl->m_Stats[STAT_HEALTH] = 100;
        g_LogFile.ss()  << "\nGirl vs. Brothel owner: " << girl->m_Realname << " is incorporial, so she wins.\n";
        g_LogFile.ssend();
        return true;
    }
    
    u_int attack = SKILL_COMBAT;    // determined later, defaults to combat
    u_int pattack = SKILL_COMBAT;
    int dodge = 0;
    int pdodge = m_Player.m_Stats[STAT_AGILITY];
    int pHealth = 100;
    int pMana = 100;
    
    // first determine what she will fight with
    if( g_Girls.GetSkill( girl, SKILL_COMBAT ) >= g_Girls.GetSkill( girl, SKILL_MAGIC ) )
        attack = SKILL_COMBAT;
    else
        attack = SKILL_MAGIC;
        
    // determine what player will fight with
    if( m_Player.m_Skills[SKILL_COMBAT] >= m_Player.m_Skills[SKILL_MAGIC] )
        pattack = SKILL_COMBAT;
    else
        pattack = SKILL_MAGIC;
        
    // calculate the girls dodge ability
    if( ( g_Girls.GetStat( girl, STAT_AGILITY ) - g_Girls.GetStat( girl, STAT_TIREDNESS ) ) < 0 )
        dodge = 0;
    else
        dodge = ( g_Girls.GetStat( girl, STAT_AGILITY ) - g_Girls.GetStat( girl, STAT_TIREDNESS ) );
        
    while( g_Girls.GetStat( girl, STAT_HEALTH ) > 20 && pHealth > 0 )
    {
        // Girl attacks
        if( ( g_Dice % 100 ) + 1 < g_Girls.GetSkill( girl, attack ) )
        {
            int damage = 0;
            
            if( attack == SKILL_MAGIC )
            {
                if( g_Girls.GetStat( girl, STAT_MANA ) <= 0 )
                {
                    attack = SKILL_COMBAT;
                    damage = 2;
                }
                else
                {
                    damage = 2 + ( g_Girls.GetSkill( girl, attack ) / 5 );
                    g_Girls.UpdateStat( girl, STAT_MANA, -7 );
                }
            }
            else
            {
                // she has hit now calculate how much damage will be done
                damage = 5 + ( g_Girls.GetSkill( girl, attack ) / 10 );
            }
            
            g_Girls.UpdateSkill( girl, attack, g_Dice % 2 ); // she may improve a little
            
            // player attempts Dodge
            if( ( g_Dice % 100 ) + 1 > pdodge )
                pHealth -= damage;
            else
                m_Player.m_Stats[STAT_AGILITY] += g_Dice % 2; // player may improve a little
        }
        
        // Player Attacks
        if( ( g_Dice % 100 ) + 1 < m_Player.m_Skills[pattack] )
        {
            int damage = 0;
            
            if( pattack == SKILL_MAGIC )
            {
                if( pMana <= 0 )
                {
                    pattack = SKILL_COMBAT;
                    damage = 2;
                }
                else
                {
                    damage = 2 + ( m_Player.m_Skills[pattack] / 5 );
                    pMana -= 5;
                }
            }
            else
            {
                // he has hit now calculate how much damage will be done
                damage = 5 + ( m_Player.m_Skills[pattack] / 10 );
            }
            
            m_Player.m_Skills[pattack] += g_Dice % 2; // he may improve a little
            
            // girl attempts Dodge
            if( ( g_Dice % 100 ) + 1 > dodge )
                g_Girls.UpdateStat( girl, STAT_HEALTH, -damage );
            else
            {
                m_Player.m_Stats[STAT_AGILITY] += g_Dice % 2; // player may improve a little
                
                if( m_Player.m_Stats[STAT_AGILITY] > 100 )
                    m_Player.m_Stats[STAT_AGILITY] = 100;
            }
        }
        
        
        // update girls dodge ability
        if( ( dodge - 2 ) < 0 )
            dodge = 0;
        else
            dodge -= 2;
            
        // update players dodge ability
        if( ( pdodge - 2 ) < 0 )
            pdodge = 0;
        else
            pdodge -= 2;
    }
    
    if( g_Girls.GetStat( girl, STAT_HEALTH ) < 20 )
    {
        g_Girls.UpdateEnjoyment( girl, ACTION_COMBAT, -1, true );
        return false;
    }
    
    g_Girls.UpdateEnjoyment( girl, ACTION_COMBAT, +1, true );
    
    return true;
}

bool BrothelManager::FightsBack( Girl* girl )
{
    if( g_Girls.GetStat( girl, STAT_HEALTH ) < 10 ) // she won't fight back in this state
        return false;
        
    if( g_Girls.HasTrait( girl, "Merciless" ) && ( ( g_Dice % 100 ) <= 20 ) )
        return true;
        
    if( g_Girls.DisobeyCheck( girl, ACTION_COMBAT ) )
        return true;
        
    return false;
}

void BrothelManager::sort( Brothel* brothel )
{
    brothel->m_Girls = girl_sort( brothel->m_Girls, &brothel->m_LastGirl );
}

bool BrothelManager::NameExists( std::string name )
{
    Brothel* current = m_Parent;
    
    while( current )
    {
        Girl* currentGirl = current->m_Girls;
        
        while( currentGirl )
        {
            if( name == currentGirl->m_Realname )
                return true;
                
            currentGirl = currentGirl->m_Next;
        }
        
        current = current->m_Next;
    }
    
    return false;
}

Girl* BrothelManager::GetFirstRunaway()
{
    return m_Runaways;
}

void BrothelManager::RemoveGirlFromRunaways( Girl* girl )
{
    if( girl->m_Next )
        girl->m_Next->m_Prev = girl->m_Prev;
        
    if( girl->m_Prev )
        girl->m_Prev->m_Next = girl->m_Next;
        
    if( girl == m_Runaways )
        m_Runaways = girl->m_Next;
        
    if( girl == m_LastRunaway )
        m_LastRunaway = girl->m_Prev;
        
    girl->m_Next = nullptr;
    girl->m_Prev = nullptr;
    m_NumRunaways--;
}

void BrothelManager::AddGirlToRunaways( Girl* girl )
{
    girl->m_Prev = nullptr;
    girl->m_Next = nullptr;
    
    if( m_Runaways )
    {
        girl->m_Prev = m_LastRunaway;
        m_LastRunaway->m_Next = girl;
        m_LastRunaway = girl;
    }
    else
        m_Runaways = m_LastRunaway = girl;
        
    m_NumRunaways++;
}

bool BrothelManager::runaway_check( Brothel* brothel, Girl* girl )
{
    /*
     *  nothing interesting happens here unless the girl is miserable
     *
     *  WD: added m_DaysUnhappy tracking
     */
    if( girl->happiness() > 10 )
    {
        girl->m_DaysUnhappy = 0;
        return false;
    }
    
    if( !girl->is_slave() )
        girl->m_DaysUnhappy++;
        
    /*
     *  now there's a matron on duty, she has a chance of fending off
     *  bad things.
     *
     *  previously, it was a 75% chance if a matron was employed
     *  so since we have two shifts, let's have a 35% chance per
     *  shift with a matron
     *
     *  with matrons being girls now, we have some opportunities
     *  for mischief here. For instance, is there still a matron skill?
     *  this should depend on that, if so. Also on how motivated the
     *  matron is. An unhappy matron, or one who hates the PC
     *  may be inclined to turn a blind eye to runaway attempts
     */
    int matron_chance = brothel->matron_count() * 35;
    
    if( g_Dice.percent( matron_chance ) )
    {
        return false;
    }
    
    /*
     *  mainly here, we're interested in the chance that she might run away
     */
    if( g_Girls.DisobeyCheck( girl, ACTION_GENERAL ) ) // check if the girl will run away
    {
        if( g_Dice.percent( m_JobManager.guard_coverage() - girl->m_DaysUnhappy ) )
        {
            return false;
        }
        
        girl->m_Events.AddMessage( "She ran away.", IMGTYPE_PROFILE, EVENT_SUMMARY );
        girl->m_DayJob = girl->m_NightJob = JOB_RUNAWAY;
        SetGirlStat( girl, STAT_TIREDNESS, 0 );
        SetGirlStat( girl, STAT_HEALTH, 100 );
        girl->m_RunAway = 6;
        std::string smess = "";
        smess += girl->m_Realname;
        smess += " has run away,";
        smess += "  Send your goons after her to attempt recapture.\nShe will escape for good after 6 weeks";
        g_MessageQue.AddToQue( smess, 1 );
        return true;
    }
    
    if( girl->m_Money <= 50 )
    {
        return false;
    }
    
    if( g_Dice.percent( 80 - girl->m_DaysUnhappy ) )
    {
        return false;
    }
    
    /*
     *  if she is unhappy she may turn to drugs
     */
    bool starts_drugs = false;
    
    if( !g_Girls.HasTrait( girl, "Shroud Addict" ) )
    {
        g_Girls.AddTrait( girl, "Shroud Addict" );
        starts_drugs = true;
    }
    else if( girl->happiness() <= 5 && !g_Girls.HasTrait( girl, "Fairy Dust Addict" ) )
    {
        g_Girls.AddTrait( girl, "Fairy Dust Addict" );
        starts_drugs = true;
    }
    else if( girl->happiness() <= 2 && !g_Girls.HasTrait( girl, "Viras Blood Addict" ) )
    {
        g_Girls.AddTrait( girl, "Viras Blood Addict" );
        starts_drugs = true;
    }
    
    /*
     *  if she Just Said No then we're done
     */
    if( !starts_drugs )
    {
        return false;
    }
    
    /*
     *  otherwise, report the sad occurrence
     */
    girl->m_Events.AddMessage(
        "This girls unhappiness has turned her onto to drugs",
        IMGTYPE_PROFILE, EVENT_WARNING
    );
    return false;
}

void BrothelManager::check_druggy_girl( std::stringstream& ss )
{
    if( g_Dice.percent( 90 ) )
        return;
        
    Girl* girl = GetDrugPossessor();
    
    if( girl == nullptr )
        return;
        
        
    ss << " They also bust a girl named "
       << girl->m_Realname
       << " for possession of drugs and send her to prison."
       ;
       
    if( g_Girls.HasTrait( girl, "Viras Blood Addict" ) )
        g_Girls.RemoveTrait( girl, "Viras Blood Addict" );
        
    if( g_Girls.HasTrait( girl, "Fairy Dust Addict" ) )
        g_Girls.RemoveTrait( girl, "Fairy Dust Addict" );
        
    if( g_Girls.HasTrait( girl, "Shroud Addict" ) )
        g_Girls.RemoveTrait( girl, "Shroud Addict" );
        
    m_NumInventory = 0;
    
    for( int i = 0; i < 40; i++ )
    {
        m_EquipedItems[i] = 0;
        m_Inventory[i] = nullptr;
    }
    
    AddGirlToPrison( girl );
}

Girl* BrothelManager::GetDrugPossessor()
{
    Brothel* current = m_Parent;
    
    while( current )
    {
        Girl* girl = current->m_Girls;
        
        while( girl )
        {
            if( ( g_Dice % 100 ) + 1 > g_Girls.GetStat( girl, STAT_INTELLIGENCE ) ) // girls will only be found out if low intelligence
            {
                if( g_Girls.HasItem( girl, "Shroud Mushroom" ) || g_Girls.HasItem( girl, "Fairy Dust" ) || g_Girls.HasItem( girl, "Vira Blood" ) )
                    return girl;
                    
                girl = girl->m_Next;
            }
        }
        
        current = current->m_Next;
    }
    
    return nullptr;
}

void BrothelManager::RemoveGirlFromPrison( Girl* girl )
{
    if( girl->m_Next )
        girl->m_Next->m_Prev = girl->m_Prev;
        
    if( girl->m_Prev )
        girl->m_Prev->m_Next = girl->m_Next;
        
    if( girl == m_Prison )
        m_Prison = girl->m_Next;
        
    if( girl == m_LastPrison )
        m_LastPrison = girl->m_Prev;
        
    girl->m_Next = nullptr;
    girl->m_Prev = nullptr;
    m_NumPrison--;
}

void BrothelManager::AddGirlToPrison( Girl* girl )
{
    // remove from girl manager if she is there
    g_Girls.RemoveGirl( girl );
    
    // remove girl from brothels if she is there
    for( int i = 0; i < g_Brothels.GetNumBrothels(); i++ )
        g_Brothels.RemoveGirl( i, girl, false );
        
    girl->m_Prev = nullptr;
    girl->m_Next = nullptr;
    
    if( m_Prison )
    {
        girl->m_Prev = m_LastPrison;
        m_LastPrison->m_Next = girl;
        m_LastPrison = girl;
    }
    else
        m_Prison = m_LastPrison = girl;
        
    m_NumPrison++;
}

Girl* BrothelManager::WhoHasTorturerJob()
{
    /*  WD:
     *  Loops through all brothels to find first
     *  girl with JOB_TORTURER
     *
     *  NOTE: assumes that only one girl, the first
     *  found is the torturer.
     *
     */
    
    Brothel* curBrothel = m_Parent;
    Girl* curGirl;
    
    while( curBrothel )                             // WD: loop through all brothels
    {
        curGirl = curBrothel->m_Girls;
        
        while( curGirl )                            // WD: loop through all girls in this brothels
        {
            if( curGirl->m_DayJob == JOB_TORTURER ) // WD: Found
            {
                //LastTortureGirl = curGirl;
                //return LastTortureGirl;
                return curGirl;
            }
            
            curGirl = curGirl->m_Next;
        }
        
        curBrothel = curBrothel->m_Next;
    }
    
    return nullptr;                                       // WD: Not Found
}

#if 0

/*
 * returns the number of customers the bar draws in
 */
int BrothelManager::bar_update( Brothel* brothel )
{
    ctariff tariff;
    
//  brothel->m_Finance.bar_upkeep(
//      tariff.empty_bar_cost()
//  );
//  if(GetAlcohol() == 0 && !m_KeepAlcStocked) {
//      if(!brothel->m_HasBarStaff) {
//          return 0;
//      }
    brothel->m_Finance.staff_wages(
        tariff.bar_staff_wages()
    );
//      return 0;
//  }
//*
// *    get the number of girls working day and night
// *
//  int day_girls = count_bar_girls(brothel, SHIFT_DAY);
//  int night_girls = count_bar_girls(brothel, SHIFT_NIGHT);
//*
// *    get the shifts for which the bar is staffed
// *
//  int shifts = 0;
//  if(brothel->m_HasBarStaff == 1) {
//      shifts = 2;
//  }
//  else {
//      if(day_girls > 2) shifts ++;
//      if(night_girls > 2) shifts ++;
//  }
//  if(shifts == 0) {
//      return 0;
//  }
//*
// *    work out the additional upkeep for running bar
// *    half price if you only run it one shift
// *
//  brothel->m_Finance.bar_upkeep(
//      tariff.active_bar_cost(brothel->m_Bar, shifts)
//  );
//
//  int maxCust = brothel->m_Bar*8;
//  int numCusts = g_Dice.random(maxCust) + day_girls + night_girls + 1;
//*
// *    loop through the customes, using up booze and adding income
// *
//  bool message_shown = false;
//  for(int i=0; i<numCusts; i++)
//  {
//      if(UseAlcohol())
//      {
//          // add the income
//          brothel->m_Finance.bar_income(g_Dice.random(20)+60);
//          brothel->m_Happiness += 100;
//          continue;
//      }
//      brothel->m_Happiness += 5;
//      if(!message_shown)
//      {
//          g_MessageQue.AddToQue(
//              "Your bars have run out of booze", 1
//          );
//          message_shown = true;
//      }
//  }
//*
// *    bar events - 90% of the time, nothing happens
// *
//  if(g_Dice.percent(90)) {
//      return numCusts;
//  }
//*
// *    bar room brawl - won't happen if guards on duty
// *
//  if(GetGangOnMission(MISS_GUARDING) > 0) {
//      return numCusts;
//  }
//*
// *    a brawl costs gold and booze
// *
//  long gold = (g_Dice%300)+1;
//  int barrels = (g_Dice%3)+1;
//*
// *    do the bookkeeping
// *
//  m_Alcohol -= barrels*5;
//  if(m_Alcohol < 0)
//      m_Alcohol = 0;
//  g_Gold.bar_upkeep(gold);
//*
// *    format the message
// *
//  std::stringstream ss;
//  ss << "A brawl breaks out in your bar located at "
//     << brothel->m_Name
//     << ". It costs you "               << gold
//     << " gold to repair the damages and you lost " << barrels
//     << " barrels of alcohol."
//  ;
//  g_MessageQue.AddToQue(ss.str(), 2);
//  return numCusts;
}

/*
 * returns the number of customers the bar draws in
 */
int BrothelManager::casino_update( Brothel* brothel )
{
    cTariff tariff;
    
    /*
     *  if the casino is staffed, it costs money
     */
    if( brothel->m_HasGambStaff )
    {
        brothel->m_Finance.casino_upkeep(
            tariff.empty_casino_cost(
                brothel->m_GamblingHall
            )
        );
    }
    
    int day_girls = count_casino_girls( brothel, SHIFT_DAY );
    int night_girls = count_casino_girls( brothel, SHIFT_NIGHT );
    /*
     *  get the shifts for which the casino is staffed
     */
    int shifts = 0;
    
    if( brothel->m_HasGambStaff == 1 )
    {
        shifts = 2;
    }
    else
    {
        if( day_girls > 4 )   shifts ++;
        
        if( night_girls > 4 ) shifts ++;
    }
    
    /*
     *  if there's no-one on either shift, we can go home
     */
    if( shifts == 0 )
    {
        return 0;
    }
    
    /*
     *  if the casino is operational, there are overheads
     *  above and beyond basic long term maintenance
     */
    brothel->m_Finance.building_upkeep(
        tariff.active_casino_cost( brothel->m_GamblingHall, shifts )
    );
    
    
    int count = 0;
    int random_range = 0;
    int numCusts = 2;
    int odds = GetGamblingChances();
    
    if( odds > 0 )
    {
        count = ( odds / 5 ) + 1;   // the amount of customer attracted is proportional to the chance of winning
        random_range = brothel->m_GamblingHall * count;
        numCusts += g_Dice.random( random_range ) + day_girls + night_girls;
    }
    
    bool message_shown = false;
    
    for( int i = 0; i < numCusts; i++ )
    {
        if( !message_shown && GetGamblingPool() == 0 )
        {
            g_MessageQue.AddToQue( "CAUTION: The gold pool for the gambling halls is empty.", 1 );
            message_shown = true;
        }
        
        /*
         *      calculate the size of the wager (won or lost)
         *      based on values in config.xml
         */
        int wager = g_Config.gamble.base;
        wager += g_Dice.random( g_Config.gamble.spread );
        bool customer_wins = g_Dice.percent( g_Config.gamble.odds );
        
        if( customer_wins == false )    // customer lose
        {
            wager = int( wager * g_Config.gamble.house_factor );
            /*
             *          if the customer loses, wee gain some money
             *          but customer happiness goes up a little anyway.
             *          Because he likes to gamble, presumably.
             *          Fair enough: no one would run gaming tables
             *          if it made the customers miserable.
             */
            brothel->m_Happiness += 10;
            AddGamblingPool( wager );
            continue;
        }
        
        /*
         *      if the customer wins, customer happiness goes up by a lot
         */
        brothel->m_Happiness += 100;
        wager = int( wager * g_Config.gamble.customer_factor );
        /*
         *      if there's enough in the pool to cover it,
         *      pay out from the pool
         */
        int pool = GetGamblingPool();
        
        if( pool >= wager )
        {
            TakeGamblingPool( wager );
            continue;       // next punter, please!
        }
        
        /*
         *      Here, and the customer wins more than the pool contains
         *
         *      There should probably be a bit more fanfare
         *      if a customer wins more on the tables than the
         *      player can cover: "the man who broke the bank at Monte Carlo"
         *      and all that...
         *
         *      Possibly have him take a girl or two as payment
         *      (if there are slaves working in the casino)
         *      Or worst case, lose the brothel. That would be a game over
         *      but then so would going too far in the red, so it works out the
         *      same.
         *
         *      otherwise, you'd be able to buy back the brothel
         *      (after a cooldown period) and the girls would
         *      eventually show up in the slave market again
         */
        g_Gold.misc_debit( wager - pool );
        TakeGamblingPool( pool );
        
    }
    
    /*
     *  if the last run through emptied the pool
     *  tell the customer
     */
    if( !message_shown && GetGamblingPool() == 0 )
    {
        g_MessageQue.AddToQue( "CAUTION: The gold pool for the gambling halls is empty.", 1 );
        message_shown = true;
    }
    
    /*
     *  get excess from pool and place into players gold
     */
    if( GetGamblingPool() > m_GamblingHallPoolMax )
    {
        int transfer = GetGamblingPool() - m_GamblingHallPoolMax;
        TakeGamblingPool( transfer );
        brothel->m_Finance.gambling_profits( transfer );
    }
    
    return numCusts;
}

#endif

} // namespace WhoreMasterRenewal
