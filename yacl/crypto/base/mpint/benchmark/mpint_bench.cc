// Copyright 2023 Ant Group Co., Ltd.
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

#include "benchmark/benchmark.h"

#include "yacl/crypto/base/mpint/mp_int.h"

namespace yacl::crypto::bench {

static void BM_MPIntCtor(benchmark::State& state) {
  for (auto _ : state) {
    for (int64_t i = 0; i < 10000; ++i) {
      benchmark::DoNotOptimize(MPInt(i));
    }
  }
}

BENCHMARK(BM_MPIntCtor)->Unit(benchmark::kMillisecond);

}  // namespace yacl::crypto::bench
