//******************************************************************************
//******************************************************************************
#ifndef SETTINGS_H
#define SETTINGS_H

#include "logger.h"

#include <vector>
#include <string>

#include <boost/property_tree/ptree.hpp>

#define TRY(_STMNT_) try { (_STMNT_); } catch(std::exception & e) { LOG() << e.what(); }

//******************************************************************************
class Settings
{
public:
    static Settings & instance();
    bool init(const std::string & filename);

    std::vector<std::string> exchangeWallets() const;

    template<typename T> T get(std::string key) const
    {
        T value;
        TRY(value = m_pt.get<T>(key));
        return value;
    }

    // boost::property_tree::ptree & properties() { return m_pt; }
private:
    Settings();

    boost::property_tree::ptree m_pt;
};

#endif // SETTINGS_H
