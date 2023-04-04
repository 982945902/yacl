#include "keys.h"

#include <vector>

#include "gtest/gtest.h"

#include "yacl/base/exception.h"
#include "yacl/crypto/base/mpint/mp_int.h"  //yacl big number

namespace yacl::crypto::test {
class KeyTest : public testing::Test {};

TEST_F(KeyTest, Test1) {
  MPInt zero(0);
  std::unique_ptr<EcGroup> ecc_group = EcGroupFactory::Create("sm2");

  Keys keys;
  std::pair<std::unique_ptr<Keys::PublicKey>, std::unique_ptr<Keys::PrivateKey>>
      key_pair_alice = keys.GenerateKeyPair(std::move(ecc_group));

  ecc_group = EcGroupFactory::Create("sm2");

  // According to the official SM2 document
  // The hexadecimal of generator is:
  // Gx = 32C4AE2C 1F198119 5F990446 6A39C994 8FE30BBF F2660BE1 715A4589
  // 334C74C7
  // Gy = BC3736A2 F4F6779C 59BDCEE3 6B692153 D0A9877C C62A4740 02DF32E5
  // 2139F0A0

  // When converting it to decimal, we have :
  //"(22963146547237050559479531362550074578802567295341616970375194840604139615431,
  //"85132369209828568825618990617112496413088388631904505083283536607588877201568)";

  std::string generator_str =
      "(2296314654723705055947953136255007457880256729534161697037519"
      "4840604139"
      "615431, "
      "85132369209828568825618990617112496413088388631904505083283536"
      "6075888772"
      "01568)";
  EXPECT_EQ(ecc_group->GetAffinePoint(key_pair_alice.first->g).ToString(),
            generator_str);

  ecc_group = EcGroupFactory::Create("sm2");
  std::pair<std::unique_ptr<Keys::PublicKey>, std::unique_ptr<Keys::PrivateKey>>
      key_pair_bob = keys.GenerateKeyPair(std::move(ecc_group));

  ecc_group = EcGroupFactory::Create("sm2");
  std::vector<Keys::KFrag> kfrags = keys.GenerateReKey(
      std::move(ecc_group), std::move(key_pair_alice.second),
      std::move(key_pair_alice.first), std::move(key_pair_bob.first), 5, 4);

  for (int i = 0; i < 5; i++) {
    EXPECT_TRUE(kfrags[i].id > zero);
  }
}
}  // namespace yacl::crypto::test