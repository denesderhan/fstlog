//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
// MAJOR.MINOR.PATCH must follow Semantic Versioning.
#define FSTLOG_VERSION_MAJOR 0
#define FSTLOG_VERSION_MINOR 55
#define FSTLOG_VERSION_PATCH 7

#define FSTLOG_STRINGIFY_IMPL(X) #X
#define FSTLOG_STRINGIFY(X) FSTLOG_STRINGIFY_IMPL(X)

#define FSTLOG_VERSION FSTLOG_STRINGIFY(\
FSTLOG_VERSION_MAJOR.\
FSTLOG_VERSION_MINOR.\
FSTLOG_VERSION_PATCH)
