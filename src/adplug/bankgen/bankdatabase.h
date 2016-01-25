#pragma once

#include <boost/optional.hpp>

#include <array>
#include <iostream>
#include <map>
#include <memory>
#include <string>

namespace opl
{
    class SlotView;
}

namespace bankdb
{

struct SlotSettings
{
    using Ptr = std::shared_ptr<SlotSettings>;

    std::array<uint8_t, 11> data{{}};
    int8_t finetune = 0;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        ar & data;
        ar & finetune;
    }

    bool operator==(const SlotSettings& b) const;

    bool operator!=(const SlotSettings& b) const;

    bool operator<(const SlotSettings& b) const;

    void apply(opl::SlotView& slot) const;
};

struct Instrument
{
    SlotSettings::Ptr first = nullptr;
    SlotSettings::Ptr second = nullptr;
    boost::optional<uint8_t> noteOverride = boost::none;
    bool pseudo4op = false;

    std::string name1{};
    std::string name2{};

    bool operator==(const Instrument& b) const;

    bool operator<(const Instrument& b) const;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        ar & first;
        ar & second;
        ar & noteOverride;
        ar & pseudo4op;
        ar & name1;
        ar & name2;
    }

private:
    static bool ptrLess(const SlotSettings::Ptr& a, const SlotSettings::Ptr& b)
    {
        if(a == nullptr && b == nullptr)
            return false;
        else if(a == nullptr && b != nullptr)
            return true;
        else if(a != nullptr && b == nullptr)
            return false;
        else
            return *a < *b;
    }
};

struct Bank
{
    std::map<size_t, Instrument> instruments{};
    std::string description{};

    template<class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        ar & description & instruments;
    }

};

class BankDatabase
{
public:
    virtual ~BankDatabase() = default;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        ar & m_banks;
    }

    void save(const std::string& filename) const;
    void load(const std::string& filename);

    const Bank* bank(const std::string& name) const
    {
        auto it = m_banks.find(name);
        if(it == m_banks.end())
            return nullptr;
        else
            return &it->second;
    }

private:
    std::map<std::string, Bank> m_banks{};

protected:
    void registerBank(const std::string& id, const std::string& description)
    {
        if(m_banks.find(id) != m_banks.end())
            throw std::runtime_error("Bank ID already registered");

        std::cout << "New bank registered: " << id << " (" << description << ")\n";
        m_banks[id].description = description;
    }

    Instrument& addInstrument(const std::string& bankId, size_t index)
    {
        if(m_banks.find(bankId) == m_banks.end())
            throw std::runtime_error("Bank ID not registered");

        return m_banks[bankId].instruments[index];
    }

    const std::map<std::string, Bank>& banks() const noexcept
    {
        return m_banks;
    }
};

} // namespace bankdb
