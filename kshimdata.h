#ifndef KSHIMDATA_H
#define KSHIMDATA_H

#include <string>
#include <vector>

class KShimData
{
public:
    KShimData();

    std::string app() const;
    void setApp(const std::string &app);

    const std::vector<std::string> &args() const;
    void setArgs(const std::vector<std::string> &args);

    std::string formatCommand(const std::vector<std::string> &args) const;

    bool isShim() const;
    const std::vector<char> &rawData() const;

    std::string toJson() const;
private:
    std::string quote(const std::string &arg) const;
    std::string quoteArgs(std::vector<std::string> args) const;
    std::string makeAbsouteCommand(const std::string &_path) const;


    std::string m_app;
    std::vector<std::string> m_args;
    std::vector<char> m_rawData;

};

#endif // KSHIMDATA_H
