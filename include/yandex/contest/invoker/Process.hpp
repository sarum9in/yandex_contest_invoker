#pragma once

#include "yandex/contest/invoker/Forward.hpp"

#include "yandex/contest/invoker/process/Result.hpp"
#include "yandex/contest/invoker/process/ResourceLimits.hpp"

#include "yandex/contest/invoker/detail/CommonProcessTypedefs.hpp"

#include "yandex/contest/invoker/detail/execution/AsyncProcessGroup.hpp"

#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>

namespace yandex{namespace contest{namespace invoker
{
    namespace process
    {
        class DefaultSettings;
    }

    /*!
     * \todo This class may copyable.
     * ProcessPointer is possibly excess class.
     * It should be checked. -- ProcessHandle
     */
    class Process: private boost::noncopyable
    {
    public:
        typedef process::Result Result;
        typedef process::ResourceLimits ResourceLimits;
        typedef process::ResourceUsage ResourceUsage;
        typedef process::DefaultSettings DefaultSettings;

    public:
        /*!
         * \brief Create new Process, associated with processGroup.
         * TODO: lifetime
         */
        static ProcessPointer create(const ProcessGroupPointer &processGroup, const std::size_t id);

        const boost::filesystem::path &executable() const;

        /*!
         * ProcessGroup is running while at least
         * on of processes marked with that flag
         * is running.
         */
        bool groupWaitsForTermination() const;

        /*!
         * \see groupWaitsForTermination()
         * \note Default value is true.
         */
        void setGroupWaitsForTermination(const bool groupWaitsForTermination=true);

        /*!
         * ProcessGroup will terminate
         * if one of processes marked with this flag
         * has crashed (completion status is not OK).
         */
        bool terminateGroupOnCrash() const;

        /*!
         * \see terminateGroupOnCrash()
         * \note Default value is true.
         */
        void setTerminateGroupOnCrash(const bool terminateGroupOnCrash=true);

        const ProcessArguments &arguments() const;
        void setArguments(const ProcessArguments &arguments);

        const boost::filesystem::path &currentPath() const;
        void setCurrentPath(const boost::filesystem::path &currentPath);

        const ProcessEnvironment &environment() const;
        void setEnvironment(const ProcessEnvironment &environment);

        const ResourceLimits &resourceLimits() const;
        void setResourceLimits(const ResourceLimits &resourceLimits);

        const system::unistd::access::Id &ownerId() const;
        void setOwnerId(const system::unistd::access::Id &ownerId);

        /*!
         * \return Process::Result previously set by ProcessGroup::poll() or ProcessGroup::wait().
         *
         * \throws ContainerIllegalStateError if process result was not set.
         *
         * \todo It is possible to implement poll() and wait() for Process. Do we need it?
         */
        const Result &result() const;

        /*!
         * \brief Assign descriptor to stream.
         */
        void setStream(const int descriptor, const Stream &stream);

        /*!
         * \brief Assign descriptor to non-pipe stream.
         */
        void setNonPipeStream(const int descriptor, const NonPipeStream &stream);

        /*!
         * \brief Get assigned stream.
         *
         * \throws std::out_of_range if no assigned stream exists.
         */
        Stream stream(const int descriptor) const;

        /*!
         * \brief Close descriptor.
         */
        void closeStream(const int descriptor);

        /*!
         * \brief If descriptor has assigned stream.
         */
        bool hasStream(const int descriptor) const;

    private:
        /*!
         * \warning Constructor is private because
         * class uses own reference-counting mechanism.
         */
        explicit Process(const ProcessGroupPointer &processGroup, const std::size_t id);

    private:
        friend void intrusive_ptr_add_ref(Process *) noexcept;
        friend void intrusive_ptr_release(Process *) noexcept;
        std::size_t refCount_ = 0;

    private:
        ProcessGroupPointer processGroup_;
        const std::size_t id_;
    };
}}}
