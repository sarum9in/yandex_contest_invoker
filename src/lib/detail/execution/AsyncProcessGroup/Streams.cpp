#include "Streams.hpp"

#include "yandex/contest/system/unistd/Operations.hpp"

#include <boost/filesystem/operations.hpp>

#include <fcntl.h>

namespace yandex{namespace contest{namespace invoker{
    namespace detail{namespace execution{namespace async_process_group_detail
{
    int Streams::operator()(const AsyncProcessGroup::File &file) const
    {
        int flags = 0;
        switch (file.accessMode)
        {
        case AsyncProcessGroup::AccessMode::READ_ONLY:
            flags |= O_RDONLY;
            break;
        case AsyncProcessGroup::AccessMode::WRITE_ONLY:
            flags |= O_WRONLY | O_TRUNC | O_CREAT;
            break;
        case AsyncProcessGroup::AccessMode::READ_WRITE:
            flags |= O_RDWR;
            break;
        }
        const boost::filesystem::path path = boost::filesystem::absolute(file.path, currentPath_);
        allocatedFDs_->push_back(system::unistd::open(path, flags, 0666));
        return allocatedFDs_->back().get();
    }

    int Streams::operator()(const AsyncProcessGroup::Pipe::End &pipeEnd) const
    {
        BOOST_ASSERT(pipeEnd.pipeId < pipes_->size());
        system::unistd::Pipe &pipe = pipes_->at(pipeEnd.pipeId);
        switch (pipeEnd.end)
        {
        case AsyncProcessGroup::Pipe::End::Type::READ:
            return pipe.readEnd();
            break;
        case AsyncProcessGroup::Pipe::End::Type::WRITE:
            return pipe.writeEnd();
            break;
        }
        BOOST_ASSERT_MSG(false, "It is impossible to get here.");
        return -1;
    }

    int Streams::operator()(const AsyncProcessGroup::FDAlias &fdAlias) const
    {
        const auto iter = descriptors_->find(fdAlias.fd);
        BOOST_ASSERT(iter != descriptors_->end());
        allocatedFDs_->push_back(system::unistd::dup(iter->second));
        return allocatedFDs_->back().get();
    }

    bool Streams::isAlias (const AsyncProcessGroup::Stream &stream) const
    {
        return boost::get<const AsyncProcessGroup::FDAlias>(&stream);
    }

    int Streams::getFD(const AsyncProcessGroup::Stream &stream) const
    {
        return boost::apply_visitor(*this, stream);
    }
}}}}}}
