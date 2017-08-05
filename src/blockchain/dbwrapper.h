#ifndef DB_WRAPPER_H
#define DB_WRAPPER_H

#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include "buff_stream.h"

class dbwrapper;

class db_batch
{
    friend class dbwrapper;
public:
    db_batch(const dbwrapper &_parent)
        : parent(_parent), size_estimate_(0) { };

    void clear()
    {
        batch.Clear();
        size_estimate_ = 0;
    }

    template <typename K, typename V>
    void write(const K& key, const V& value)
    {
        ssKey << key;
        leveldb::Slice slKey(ssKey.data(), ssKey.size());
        ssValue << value;
        leveldb::Slice slValue(ssValue.data(), ssValue.size());

        batch.Put(slKey, slValue);

        size_estimate_ += 3 + (slKey.size() > 127) + slKey.size() + (slValue.size() > 127) + slValue.size();
        ssKey.clear();
        ssValue.clear();
    }

    template <typename K>
    void erase(const K& key)
    {
        ssKey << key;
        leveldb::Slice slKey(ssKey.data(), ssKey.size());

        batch.Delete(slKey);

        size_estimate_ += 2 + (slKey.size() > 127) + slKey.size();
        ssKey.clear();
    }

    size_t size_estimate() const { return size_estimate_; }

private:
    const dbwrapper &parent;
    leveldb::WriteBatch batch;

    buff_stream ssKey;
    buff_stream ssValue;

    size_t size_estimate_;
};

class db_iterator
{
public:
    db_iterator(const dbwrapper &_parent, leveldb::Iterator *_piter)
        : parent(_parent), piter(_piter) { };
    ~db_iterator() { delete piter; }

    bool valid() { return piter->Valid(); }

    void seek_to_first() { piter->SeekToFirst(); }

    template<typename K> void seek(const K& key)
    {
        buff_stream ssKey;
        ssKey << key;
        leveldb::Slice slKey(ssKey.data(), ssKey.size());
        piter->Seek(slKey);
    }

    void next() { piter->Next(); }

    template<typename K> bool get_key(K& key)
    {
        leveldb::Slice slKey = piter->key();
        try
        {
            buff_stream ssKey(slKey.data(), slKey.data() + slKey.size());
            ssKey >> key;
        }
        catch (const std::exception&)
        {
            return false;
        }
        return true;
    }

    template<typename V> bool get_value(V& value)
    {
        leveldb::Slice slValue = piter->value();
        try
        {
            buff_stream ssValue(slValue.data(), slValue.data() + slValue.size());
            ssValue >> value;
        }
        catch (const std::exception&)
        {
            return false;
        }
        return true;
    }

    unsigned int get_value_size()
    {
        return piter->value().size();
    }

private:
    const dbwrapper &parent;
    leveldb::Iterator *piter;
};

class dbwrapper
{
public:
    dbwrapper(const char *path);
    ~dbwrapper();

    template <typename K, typename V>
    bool read(const K& key, V& value) const
    {
        buff_stream ssKey;
        ssKey << key;
        leveldb::Slice slKey(ssKey.data(), ssKey.size());

        std::string strValue;
        leveldb::Status status = pdb->Get(readoptions, slKey, &strValue);
        if (!status.ok())
        {
            if (status.IsNotFound())
                return false;
            handle_db_error(status);
        }
        try
        {
            buff_stream ssValue(strValue.data(), strValue.data() + strValue.size());
            ssValue >> value;
        }
        catch (const std::exception&)
        {
            return false;
        }
        return true;
    }

    template <typename K, typename V>
    bool write(const K& key, const V& value, bool fSync = false)
    {
        db_batch batch(*this);
        batch.write(key, value);
        return write_batch(batch, fSync);
    }

    template <typename K>
    bool exists(const K& key) const
    {
        buff_stream ssKey;
        ssKey << key;
        leveldb::Slice slKey(ssKey.data(), ssKey.size());

        std::string strValue;
        leveldb::Status status = pdb->Get(readoptions, slKey, &strValue);
        if (!status.ok())
        {
            if (status.IsNotFound())
                return false;
            handle_db_error(status);
        }
        return true;
    }

    template <typename K>
    bool erase(const K& key, bool fSync = false)
    {
        db_batch batch(*this);
        batch.erase(key);
        return write_batch(batch, fSync);
    }

    bool write_batch(db_batch& batch, bool fSync = false);

    db_iterator *new_iterator()
    {
        return new db_iterator(*this, pdb->NewIterator(iteroptions));
    }

    static void handle_db_error(const leveldb::Status& status);
private:
    leveldb::Options options;
    leveldb::ReadOptions readoptions;
    leveldb::ReadOptions iteroptions;
    leveldb::WriteOptions writeoptions;
    leveldb::WriteOptions syncoptions;
    leveldb::DB* pdb; 
};

#endif // DB_WRAPPER_H
