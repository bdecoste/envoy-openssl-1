#include "extensions/common/crypto/crypto_impl.h"

#include <atomic>
#include <cstdint>
#include <list>

namespace Envoy {
namespace Common {
namespace Crypto {

EVP_PKEY* PublicKeyWrapper::getEVP_PKEY() const { return pkey_.get(); }

void PublicKeyWrapper::setEVP_PKEY(EVP_PKEY* pkey) { pkey_.reset(pkey); }

} // namespace Crypto
} // namespace Common
} // namespace Envoy
