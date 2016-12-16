/*! \file
 *
 * \brief File checkpointing backend
 * \author Benjamin Pritchard (ben@bennyp.org)
 */


#include "pulsar/modulemanager/checkpoint_backends/FileCheckpointIO.hpp"
#include "pulsar/util/Serialization.hpp"
#include "pulsar/exception/Exceptions.hpp"
#include "pulsar/output/GlobalOutput.hpp"

using namespace pulsar::exception;

namespace {


}



namespace pulsar {
namespace modulemanager {
        

FileCheckpointIO::FileCheckpointIO(const std::string & path, bool truncate)
    : path_(path)
{
    using pulsar::util::from_byte_array;

    bool newfile = truncate;

    // does the file exist?
    auto flags = std::fstream::in | std::fstream::out | std::fstream::binary;
    if(truncate)
        flags |= std::fstream::trunc;

    file_.open(path.c_str(), flags);

    if(!file_.is_open() && !truncate)
    {
        // file (probably) doesn't exist. So try again with fstream::trunc
        // which is ok on an empty file
        file_.open(path.c_str(), flags | std::fstream::trunc);
        newfile = true;
    }

    if(!file_.is_open())
        throw GeneralException("File could not be opened for reading and writing", "path", path);




    if(!newfile)
    {
        // read existing data
        file_.seekg(0, std::fstream::beg);

        while(!file_.eof())
        {
            // read in the size of the toc entry
            size_t toc_size;
            file_.read(reinterpret_cast<char *>(&toc_size), sizeof(size_t));

            // read in the byte array for the toc entry
            ByteArray toc_bytes(toc_size);
            file_.read(toc_bytes.data(), toc_size);

            // unserialize the TOC entry
            FileTOCEntry toc_entry = from_byte_array<FileTOCEntry>(toc_bytes);

            // skip ahead
            file_.seekg(toc_entry.metadata_size + toc_entry.data_size, std::fstream::cur);

            // place in my toc
            toc_.emplace(toc_entry.key, toc_entry);
            output::print_global_warning("Read entry %?\n", toc_entry.key);
        }

        // clear the eof flag
        file_.clear();

        output::print_global_warning("Read %? entries from 5%?\n", size(), path);
    }


    // the file will throw exceptions if there is a problem
    file_.exceptions(std::fstream::failbit | std::fstream::badbit);
}


size_t FileCheckpointIO::size(void) const
{
    return toc_.size();
}


size_t FileCheckpointIO::count(const std::string & key) const
{
    return toc_.count(key);
}

std::set<std::string> FileCheckpointIO::all_keys(void) const
{
    std::set<std::string> keys;
    for(const auto & it : toc_)
        keys.insert(it.first);
    return keys;
}

void FileCheckpointIO::write(const std::string & key,
                             const ByteArray & metadata,
                             const ByteArray & data)
{
    using pulsar::util::to_byte_array;

    // seek to the end of the file
    file_.seekp(0, std::fstream::end);

    std::streampos curpos = file_.tellp();

    // make a TOC entry & serialize it
    FileTOCEntry toc_entry{key, metadata.size(), data.size(), curpos};
    ByteArray toc_entry_bytes = to_byte_array(toc_entry);
    size_t toc_entry_size = toc_entry_bytes.size();

    output::print_global_warning("toc entry size: %?\n", toc_entry_size);
    file_.write(reinterpret_cast<const char *>(&toc_entry_size), sizeof(toc_entry_size));
    file_.write(toc_entry_bytes.data(), toc_entry_size);


    // write the actual data
    file_.write(metadata.data(), metadata.size());
    file_.write(data.data(), data.size());

    // place in my toc
    toc_.emplace(toc_entry.key, toc_entry);
}

std::pair<ByteArray, ByteArray> FileCheckpointIO::read(const std::string & key) const
{
    if(!toc_.count(key))
        throw GeneralException("Cannot read metadata from checkpoint file - key doesn't exist",
                               "key", key);

    const auto & toc_entry = toc_.at(key);

    // seek to where the data is
    file_.seekg(toc_entry.pos, std::fstream::beg);

    // read the size of the toc entry and skip ahead, since we already have it
    size_t toc_size;
    file_.read(reinterpret_cast<char *>(&toc_size), sizeof(size_t));
    file_.seekg(toc_size, std::fstream::cur);

    std::pair<ByteArray, ByteArray> alldata_bytes;
    alldata_bytes.first.resize(toc_entry.metadata_size);
    alldata_bytes.second.resize(toc_entry.data_size);

    file_.read(alldata_bytes.first.data(), toc_entry.metadata_size);
    file_.read(alldata_bytes.second.data(), toc_entry.data_size);
    return alldata_bytes;
}

ByteArray FileCheckpointIO::read_metadata(const std::string & key) const
{
    if(!toc_.count(key))
        throw GeneralException("Cannot read metadata from checkpoint file - key doesn't exist",
                               "key", key);

    const auto & toc_entry = toc_.at(key);

    // seek to where the data is
    file_.seekg(toc_entry.pos, std::fstream::beg);

    // read the size of the toc entry and skip ahead, since we already have it
    size_t toc_size;
    file_.read(reinterpret_cast<char *>(&toc_size), sizeof(size_t));
    file_.seekg(toc_size, std::fstream::cur);

    ByteArray metadata_bytes(toc_entry.metadata_size);
    file_.read(metadata_bytes.data(), toc_entry.metadata_size);
    return metadata_bytes;
}

void FileCheckpointIO::erase(const std::string & key)
{
}

void FileCheckpointIO::clear(void)
{
}


} // close namespace modulemanager
} // close namespace pulsar

