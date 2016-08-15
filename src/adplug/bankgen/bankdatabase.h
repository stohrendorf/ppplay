#pragma once

#include <boost/array.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/serialization/tracking.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/version.hpp>

#include <iostream>
#include <map>
#include <string>

namespace opl
{
    class SlotView;
}

namespace bankdb
{

struct SlotSettings
{
    using Ptr = boost::shared_ptr<SlotSettings>;

    boost::array<uint8_t, 11> data{{}};
    int8_t finetune = 0;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        ar & BOOST_SERIALIZATION_NVP(data);
        ar & BOOST_SERIALIZATION_NVP(finetune);
    }

    bool operator==(const SlotSettings& b) const;

    bool operator!=(const SlotSettings& b) const;

    bool operator<(const SlotSettings& b) const;

    void apply(opl::SlotView& slot) const;

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

struct Instrument
{
    SlotSettings::Ptr first = nullptr;
    SlotSettings::Ptr second = nullptr;
    boost::optional<uint8_t> noteOverride = boost::none;
    bool pseudo4op = false;

    std::string localName{};
    std::string generatedName{};

    bool operator==(const Instrument& b) const;

    bool operator<(const Instrument& b) const;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        ar & BOOST_SERIALIZATION_NVP(first);
        ar & BOOST_SERIALIZATION_NVP(second);
        ar & BOOST_SERIALIZATION_NVP(noteOverride);
        ar & BOOST_SERIALIZATION_NVP(pseudo4op);
        ar & BOOST_SERIALIZATION_NVP(localName);
        ar & BOOST_SERIALIZATION_NVP(generatedName);
    }
};

struct Bank
{
    std::map<size_t, Instrument> instruments{};
    std::string description{};
    bool uses4op = false;
    bool onlyPercussion = false;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        ar & BOOST_SERIALIZATION_NVP(description);
        ar & BOOST_SERIALIZATION_NVP(instruments);
        ar & BOOST_SERIALIZATION_NVP(uses4op);
    }

};

class BankDatabase
{
public:
    virtual ~BankDatabase() = default;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        ar & boost::serialization::make_nvp("banks", m_banks);
    }

    void load(const std::string& filename);

    const Bank* bank(const std::string& name) const
    {
        auto it = m_banks.find(name);
        if(it == m_banks.end())
            return nullptr;
        else
            return &it->second;
    }

    const std::map<std::string, Bank>& banks() const noexcept
    {
        return m_banks;
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

    std::map<std::string, Bank>& banks() noexcept
    {
        return m_banks;
    }
};

} // namespace bankdb

BOOST_CLASS_TRACKING(bankdb::SlotSettings, boost::serialization::track_always)
BOOST_CLASS_VERSION(bankdb::SlotSettings, 1)
BOOST_CLASS_VERSION(bankdb::Instrument, 1)
BOOST_CLASS_VERSION(bankdb::BankDatabase, 1)
