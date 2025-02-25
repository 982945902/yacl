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

#include <memory>
#include <string>

#include "fmt/format.h"
#include "gtest/gtest.h"
#include "sodium/crypto_scalarmult_ed25519.h"

#include "yacl/crypto/base/ecc/ec_point.h"
#include "yacl/crypto/base/ecc/ecc_spi.h"
#include "yacl/crypto/base/ecc/libsodium/ed25519_group.h"

namespace yacl::crypto::sodium {
MPInt Fe25519ToMPInt(const fe25519& x);
void MPIntToFe25519(const MPInt& x, fe25519* out);
}  // namespace yacl::crypto::sodium

namespace yacl::crypto::sodium::test {

std::ostream& operator<<(std::ostream& os, const EcPoint& p) {
  const auto* p3 =
      reinterpret_cast<const ge25519_p3*>(std::get<Array160>(p).data());

  return os << fmt::format("[X={}, Y={}, Z={}, T={}]", Fe25519ToMPInt(p3->X),
                           Fe25519ToMPInt(p3->Y), Fe25519ToMPInt(p3->Z),
                           Fe25519ToMPInt(p3->T));
}

class SodiumTest : public ::testing::Test {
 protected:
  std::unique_ptr<EcGroup> ec_ = EcGroupFactory::Create("ed25519", "libsodium");
};

TEST_F(SodiumTest, CopyPointWorks) {
  // Convert function
  fe25519 f;
  MPIntToFe25519(123456789_mp, &f);
  EXPECT_EQ(Fe25519ToMPInt(f), 123456789_mp);

  // CopyPoint: ge25519_p3 -> ge25519_p3
  auto ecp1 = ec_->MulBase(852_mp);
  auto ecp2 = ec_->CopyPoint(ecp1);
  EXPECT_TRUE(ec_->PointEqual(ecp1, ecp2));

  // check is deep copy
  ec_->AddInplace(&ecp1, ec_->GetGenerator());
  EXPECT_FALSE(ec_->PointEqual(ecp1, ecp2));

  // CopyPoint: affine_point -> ge25519_p3
  auto ap = ec_->GetAffinePoint(ecp1);
  EXPECT_TRUE(ec_->PointEqual(ecp1, ec_->CopyPoint(ap)));

  // CopyPoint: compressed point -> ge25519_p3
  Array32 scalar;
  (853_mp).ToBytes(scalar.data(), scalar.size(), Endian::little);
  Array32 buf;
  crypto_scalarmult_ed25519_base_noclamp(buf.data(), scalar.data());
  auto ecp3 = ec_->CopyPoint(buf);
  EXPECT_TRUE(ec_->PointEqual(ecp1, ecp3));
}

TEST_F(SodiumTest, AffinePointWorks) {
  ASSERT_EQ(ec_->GetLibraryName(), "libsodium");

  AffinePoint ap_g{
      "0x216936D3CD6E53FEC0A4E231FDD6DC5C692CC7609525A7B2C9562D608F25D51A"_mp,
      "0x6666666666666666666666666666666666666666666666666666666666666658"_mp};
  auto g = ec_->GetAffinePoint(ec_->GetGenerator());
  EXPECT_EQ(ap_g, g);

  EcPoint new_g = ec_->CopyPoint(ap_g);
  EXPECT_TRUE(ec_->PointEqual(new_g, ec_->GetGenerator()));

  auto inf = ec_->GetAffinePoint(ec_->MulBase(0_mp));
  EXPECT_EQ(inf, AffinePoint(0_mp, 1_mp)) << inf;
  EXPECT_TRUE(ec_->PointEqual(ec_->CopyPoint(inf), ec_->MulBase(0_mp)));

  auto any_p = ec_->MulBase(123456_mp);
  auto any_ap = ec_->GetAffinePoint(any_p);
  EXPECT_TRUE(ec_->PointEqual(ec_->CopyPoint(any_ap), any_p));
}

TEST_F(SodiumTest, InfWorks) {
  auto g = ec_->GetGenerator();
  EXPECT_TRUE(ec_->IsInCurveGroup(g));

  auto inf = ec_->MulBase(0_mp);
  auto inf2 = ec_->CopyPoint(inf);
  EXPECT_TRUE(ec_->PointEqual(inf, inf2));
  EXPECT_FALSE(ec_->PointEqual(inf, g));
  EXPECT_TRUE(ec_->IsInfinity(inf2));
  EXPECT_TRUE(ec_->IsInCurveGroup(inf2));

  ec_->AddInplace(&inf, ec_->GetGenerator());
  EXPECT_FALSE(ec_->PointEqual(inf, inf2));
  EXPECT_TRUE(ec_->PointEqual(inf, g));
}

TEST_F(SodiumTest, NegateWorks) {
  // simple works
  auto g = ec_->MulBase(1_mp);
  EXPECT_TRUE(ec_->PointEqual(g, ec_->GetGenerator()));
  auto g2 = ec_->MulBase(2_mp);
  EXPECT_TRUE(ec_->PointEqual(g2, ec_->Add(g, g)));

  EcPoint ng = ec_->Negate(ec_->GetGenerator());
  EXPECT_TRUE(ec_->PointEqual(ng, ec_->MulBase(-1_mp)));
  EXPECT_TRUE(ec_->PointEqual(ng, ec_->Sub(g, g2)));
  auto inf = ec_->Add(ng, ec_->GetGenerator());
  EXPECT_TRUE(ec_->IsInfinity(inf));
  ec_->NegateInplace(&inf);
  EXPECT_TRUE(ec_->IsInfinity(inf));
  EXPECT_EQ(ec_->GetAffinePoint(inf), AffinePoint(0_mp, 1_mp));

  EXPECT_TRUE(ec_->PointEqual(ec_->MulBase(-2_mp), ec_->Add(ng, ng)));
  EXPECT_TRUE(ec_->PointEqual(ec_->MulBase(-1000_mp),
                              ec_->Negate(ec_->MulBase(1000_mp))));
}

}  // namespace yacl::crypto::sodium::test
