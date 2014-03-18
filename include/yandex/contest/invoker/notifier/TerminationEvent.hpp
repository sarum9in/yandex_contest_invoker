#pragma once

#include <yandex/contest/invoker/notifier/ProcessId.hpp>
#include <yandex/contest/invoker/process/Result.hpp>

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

namespace yandex{namespace contest{namespace invoker{namespace notifier
{
    struct TerminationEvent
    {
        friend class boost::serialization::access;

        template <typename Archive>
        void serialize(Archive &ar, const unsigned int)
        {
            ar & BOOST_SERIALIZATION_NVP(processId);
            ar & BOOST_SERIALIZATION_NVP(result);
        }

        ProcessId processId;
        process::Result result;
    };
}}}}
