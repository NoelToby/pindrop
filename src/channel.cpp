// Copyright 2014 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "pindrop/pindrop.h"

#include "channel_internal_state.h"

namespace pindrop {

const Channel kInvalidChannel(nullptr);

const int kFadeOutDurationMs = 10;

void Channel::Clear() { state_ = nullptr; }

bool Channel::Valid() const { return state_ != nullptr; }

bool Channel::Playing() const {
  assert(Valid());
  return state_->Playing();
}

void Channel::Stop() {
  assert(Valid());
  // Fade out rather than halting to avoid clicks.  However, SDL_Mixer will
  // not fade out channels with a volume of 0.  Manually halt channels in this
  // case.
  if (state_->Gain() == 0.0f) {
    state_->Halt();
  } else {
    state_->FadeOut(kFadeOutDurationMs);
  }
}

const mathfu::Vector<float, 3> Channel::Location() const {
  assert(Valid());
  return state_->Location();
}

void Channel::SetLocation(const mathfu::Vector<float, 3>& location) {
  assert(Valid());
  state_->SetLocation(location);
}

}  // namespace pindrop
