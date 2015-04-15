//******************************************************************************
//******************************************************************************

#include "settings.h"
#include "logger.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>

//******************************************************************************
//******************************************************************************
// static
Settings & Settings::instance()
{
    static Settings s;
    return s;
}

//******************************************************************************
//******************************************************************************
Settings::Settings()
{
}

//******************************************************************************
//******************************************************************************
bool Settings::init(const std::string & filename)
{
    try
    {
        boost::property_tree::ini_parser::read_ini(filename, m_pt);
        return true;
    }
    catch (std::exception & e)
    {
        LOG() << e.what();
    }

    return false;
}

//******************************************************************************
//******************************************************************************
std::vector<std::string> Settings::exchangeWallets() const
{
    std::string list;
    TRY(list = m_pt.get<std::string>("Main.ExchangeWallets"));

    std::vector<std::string> strs;
    if (list.size() > 0)
    {
        boost::split(strs, list, boost::is_any_of(",;:"));
    }

    return strs;
}
