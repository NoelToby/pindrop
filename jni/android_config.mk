# Copyright 2014 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Locations of 3rd party and FPL libraries.
THIRD_PARTY_ROOT:=$(PINDROP_DIR)/../../../../external
FPL_ROOT:=$(PINDROP_DIR)/../../libs
# If the dependencies directory exists either as a subdirectory or as the
# container of this project directory, assume the dependencies directory is
# the root directory for all libraries required by this project.
$(foreach dep_dir,$(wildcard $(PINDROP_DIR)/dependencies) \
                  $(wildcard $(PINDROP_DIR)/../../dependencies),\
  $(eval DEPENDENCIES_ROOT?=$(dep_dir)))
ifneq ($(DEPENDENCIES_ROOT),)
  THIRD_PARTY_ROOT:=$(DEPENDENCIES_ROOT)
  FPL_ROOT:=$(DEPENDENCIES_ROOT)
endif

# Location of the SDL library.
DEPENDENCIES_SDL_DIR?=$(THIRD_PARTY_ROOT)/sdl
# Location of the SDL Mixer library.
DEPENDENCIES_SDL_MIXER_DIR?=$(THIRD_PARTY_ROOT)/sdl_mixer
# Location of the Flatbuffers library.
DEPENDENCIES_FLATBUFFERS_DIR?=$(FPL_ROOT)/flatbuffers
# Location of the googletest library.
DEPENDENCIES_GTEST_DIR?=$(FPL_ROOT)/googletest
# Location of the fplbase library.
DEPENDENCIES_FPLBASE_DIR?=$(FPL_ROOT)/fplbase
# Location of the fplutil library.
DEPENDENCIES_FPLUTIL_DIR?=$(FPL_ROOT)/fplutil

