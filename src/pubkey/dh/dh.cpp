/*
* Diffie-Hellman
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#include <botan/dh.h>
#include <botan/numthry.h>
#include <botan/internal/workfactor.h>

namespace Botan {

/*
* DH_PublicKey Constructor
*/
DH_PublicKey::DH_PublicKey(const DL_Group& grp, const BigInt& y1)
   {
   group = grp;
   y = y1;
   X509_load_hook();
   }

/*
* Algorithm Specific X.509 Initialization Code
*/
void DH_PublicKey::X509_load_hook()
   {
   }

/*
* Return the maximum input size in bits
*/
u32bit DH_PublicKey::max_input_bits() const
   {
   return group_p().bits();
   }

/*
* Return the public value for key agreement
*/
MemoryVector<byte> DH_PublicKey::public_value() const
   {
   return BigInt::encode_1363(y, group_p().bytes());
   }

/*
* Create a DH private key
*/
DH_PrivateKey::DH_PrivateKey(RandomNumberGenerator& rng,
                             const DL_Group& grp,
                             const BigInt& x_arg)
   {
   group = grp;
   x = x_arg;

   if(x == 0)
      {
      const BigInt& p = group_p();
      x.randomize(rng, 2 * dl_work_factor(p.bits()));
      }

   if(y == 0)
      y = power_mod(group_g(), x, group_p());

   core = DH_Core(rng, group, x);

   if(x == 0)
      gen_check(rng);
   else
      load_check(rng);
   }

/*
* Load a DH private key
*/
DH_PrivateKey::DH_PrivateKey(const AlgorithmIdentifier& alg_id,
                             const MemoryRegion<byte>& key_bits,
                             RandomNumberGenerator& rng) :
   DL_Scheme_PrivateKey(alg_id, key_bits, DL_Group::ANSI_X9_42)
   {
   if(y == 0)
      y = power_mod(group_g(), x, group_p());

   core = DH_Core(rng, group, x);

   load_check(rng);
   }

/*
* Return the public value for key agreement
*/
MemoryVector<byte> DH_PrivateKey::public_value() const
   {
   return DH_PublicKey::public_value();
   }

/*
* Derive a key
*/
SecureVector<byte> DH_PrivateKey::derive_key(const byte w[],
                                             u32bit w_len) const
   {
   return derive_key(BigInt::decode(w, w_len));
   }

/*
* Derive a key
*/
SecureVector<byte> DH_PrivateKey::derive_key(const DH_PublicKey& key) const
   {
   return derive_key(key.get_y());
   }

/*
* Derive a key
*/
SecureVector<byte> DH_PrivateKey::derive_key(const BigInt& w) const
   {
   const BigInt& p = group_p();
   if(w <= 1 || w >= p-1)
      throw Invalid_Argument(algo_name() + "::derive_key: Invalid key input");
   return BigInt::encode_1363(core.agree(w), p.bytes());
   }

}
