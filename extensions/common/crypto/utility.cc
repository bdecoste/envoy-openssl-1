#include "common/crypto/utility.h"

#include <iostream>

#include "common/common/assert.h"
#include "common/common/stack_array.h"

#include "extensions/common/crypto/crypto_impl.h"

#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "openssl/evp.h"
#include "openssl/hmac.h"
#include "openssl/sha.h"

namespace Envoy {
namespace Common {
namespace Crypto {

namespace Utility {

const EVP_MD* getHashFunction(absl::string_view name);

std::vector<uint8_t> getSha256Digest(const Buffer::Instance& buffer) {
  std::vector<uint8_t> digest(SHA256_DIGEST_LENGTH);
  EVP_MD_CTX* ctx(EVP_MD_CTX_new());
  auto rc = EVP_DigestInit(ctx, EVP_sha256());
  RELEASE_ASSERT(rc == 1, "Failed to init digest context");
  const auto num_slices = buffer.getRawSlices(nullptr, 0);
  STACK_ARRAY(slices, Buffer::RawSlice, num_slices);
  buffer.getRawSlices(slices.begin(), num_slices);
  for (const auto& slice : slices) {
    rc = EVP_DigestUpdate(ctx, slice.mem_, slice.len_);
    RELEASE_ASSERT(rc == 1, "Failed to update digest");
  }
  rc = EVP_DigestFinal(ctx, digest.data(), nullptr);
  RELEASE_ASSERT(rc == 1, "Failed to finalize digest");
  EVP_MD_CTX_free(ctx);
  return digest;
}

std::vector<uint8_t> getSha256Hmac(const std::vector<uint8_t>& key, absl::string_view message) {
  std::vector<uint8_t> hmac(SHA256_DIGEST_LENGTH);
  const auto ret =
      HMAC(EVP_sha256(), key.data(), key.size(), reinterpret_cast<const uint8_t*>(message.data()),
           message.size(), hmac.data(), nullptr);
  RELEASE_ASSERT(ret != nullptr, "Failed to create HMAC");
  return hmac;
}

const VerificationOutput verifySignature(absl::string_view hash, void* pubKey,
                                         const std::vector<uint8_t>& signature,
                                         const std::vector<uint8_t>& text) {
  // Step 1: initialize EVP_MD_CTX
  EVP_MD_CTX *ctx;
  ctx = EVP_MD_CTX_new();

  // Step 2: initialize EVP_MD
  const EVP_MD* md = getHashFunction(hash);

  if (md == nullptr) {
    return {false, absl::StrCat(hash, " is not supported.")};
  }

  // Step 3: initialize EVP_DigestVerify
  int ok =
      EVP_DigestVerifyInit(ctx, nullptr, md, nullptr, reinterpret_cast<EVP_PKEY*>(pubKey));
  if (!ok) {
    return {false, "Failed to initialize digest verify."};
  }

  // Step 4: verify signature
  ok = EVP_DigestVerify(ctx, signature.data(), signature.size(), text.data(), text.size());

  // Step 5: check result
  if (ok == 1) {
    return {true, ""};
  }

  return {false, absl::StrCat("Failed to verify digest. Error code: ", ok)};
}

CryptoObjectUniquePtr importPublicKey(const std::vector<uint8_t>& key) {
  const uint8_t* data = reinterpret_cast<const uint8_t*>(key.data());
  EVP_PKEY* pkey = d2i_PUBKEY(nullptr, &data, key.size());

  auto publicKeyWrapper = new PublicKeyWrapper();
  publicKeyWrapper->setEVP_PKEY(pkey);

  std::unique_ptr<PublicKeyWrapper> publicKeyPtr = std::make_unique<PublicKeyWrapper>();
  publicKeyPtr.reset(publicKeyWrapper);

  return publicKeyPtr;
}

const EVP_MD* getHashFunction(absl::string_view name) {
  const std::string hash = absl::AsciiStrToLower(name);

  // Hash algorithms set refers
  // https://github.com/google/boringssl/blob/master/include/openssl/digest.h
  if (hash == "sha1") {
    return EVP_sha1();
  } else if (hash == "sha224") {
    return EVP_sha224();
  } else if (hash == "sha256") {
    return EVP_sha256();
  } else if (hash == "sha384") {
    return EVP_sha384();
  } else if (hash == "sha512") {
    return EVP_sha512();
  } else {
    return nullptr;
  }
}

} // namespace Utility
} // namespace Crypto
} // namespace Common
} // namespace Envoy
