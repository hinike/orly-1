/* <orly/type/obj.cc>

   Implements <orly/type/obj.h>.

   Copyright 2010-2014 OrlyAtomics, Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

#include <orly/type/obj.h>

using namespace Orly::Type;

IMPL_INTERNED_TYPE(TObj, TObjElems);

TObj::~TObj() {}

void TObj::Write(std::ostream &out) const {
  assert(this);
  assert(&out);
  out << "Orly::Rt::Objects::TObj" << AsType().GetMangledName();
}