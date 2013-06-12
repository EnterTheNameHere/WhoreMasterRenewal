
#include "cJobs.h"

namespace WhoreMasterRenewal
{

sJobs::sJobs()
{
    ;
}

sJobs::~sJobs()
{
    ;
}

void sJobs::setup( std::string name,
                    int min,
                    int max,
                    bool slave,
                    bool free,
                    char girlpay,
                    char up,
                    int id,
                    std::string like,
                    std::string hate,
                    int skill )
{
    m_Name = name;
    m_LikeWork = like;
    m_HateWork = hate;
    m_DependantSkill = skill;
    m_FreeGirlAble = free;
    m_SlaveAble = slave;
    m_MinPositions = min;
    m_MaxPositions = max;
    m_Upkeep = up;
    m_GirlPay = girlpay;
    m_ActionID = id;
}

} // namespace WhoreMasterRenewal
