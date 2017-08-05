#include "dbwrapper.h"
#include <boost/filesystem.hpp>

dbwrapper::dbwrapper(const char *path)
{
    boost::filesystem::create_directories(path);
    options.create_if_missing = true;
    readoptions.verify_checksums = true;
    iteroptions.verify_checksums = true;
    iteroptions.fill_cache = false;
    syncoptions.sync = true;
    leveldb::Status status = leveldb::DB::Open(options, path, &pdb);
    handle_db_error(status);
}

dbwrapper::~dbwrapper()
{
    delete pdb;
}

bool dbwrapper::write_batch(db_batch& batch, bool fSync)
{
    leveldb::Status status = pdb->Write(fSync ? syncoptions : writeoptions, &batch.batch);
    handle_db_error(status);
    return true;
}

void dbwrapper::handle_db_error(const leveldb::Status& status)
{
    if (status.ok())
        return;
    if (status.IsCorruption())
        throw std::runtime_error("Database corrupted");
    if (status.IsIOError())
        throw std::runtime_error("Database I/O error");
    if (status.IsNotFound())
        throw std::runtime_error("Database entry missing");
    throw std::runtime_error("Unknown database error");
}
