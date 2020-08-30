:mod:`ucryptolib` -- cryptographic ciphers
==========================================

.. module:: ucryptolib
   :synopsis: cryptographic ciphers

Classes
-------

.. class:: aes

    .. classmethod:: __init__(key, mode, [IV])

        Initialize cipher object, suitable for encryption/decryption. Note:
        after initialization, cipher object can be use only either for
        encryption or decryption. Running decrypt() operation after encrypt()
        or vice versa is not supported.

        Parameters are:

            * *key* is an encryption/decryption key (bytes-like).
            * *mode* is:

                * ``1`` (or ``ucryptolib.MODE_ECB`` if it exists) for Electronic Code Book (ECB).
                * ``2`` (or ``ucryptolib.MODE_CBC`` if it exists) for Cipher Block Chaining (CBC).
                * ``6`` (or ``ucryptolib.MODE_CTR`` if it exists) for Counter mode (CTR).

            * *IV* is an initialization vector for CBC mode.
            * For Counter mode, *IV* is the initial value for the counter.

    .. method:: encrypt(in_buf, [out_buf])

        Encrypt *in_buf*. If no *out_buf* is given result is returned as a
        newly allocated `bytes` object. Otherwise, result is written into
        mutable buffer *out_buf*. *in_buf* and *out_buf* can also refer
        to the same mutable buffer, in which case data is encrypted in-place.

    .. method:: decrypt(in_buf, [out_buf])

        Like `encrypt()`, but for decryption.

.. class:: aesgcm

    .. classmethod:: __init__(key)

        Initialize a cipher object for Authenticated Encryption with
        Additional Data (AEAD) using AES in Galois Counter Mode,
        suitable for either encryption or decryption. AEAD protects
        the confidentiality of the plaintext and also protects the
        integrity of both plaintext and additional data.  Note: This
        object can be reused for both encryption and decryption
        operations in any order.
        
        Parameters are:

            * *key* is an encryption/decryption key (bytes-like).

    .. method:: encrypt(nonce, in_buf, out_buf=None, adata=None)

        Encrypt the data in the buffer *in_buf* using the one-time
        initialisation value *nonce* and append an authentication tag
        based on the data in *in_buf* and optionally the additional
        data in the buffer *adata*. If no *out_buf* is given the
        result is returned as a newly allocated `bytes`
        object. Otherwise, the result is written into mutable buffer
        *out_buf*.
        
    .. method:: decrypt(nonce, in_buf, out_buf=None, adata=None)

        Decrypt and authenticate the contents of the input buffer
        *in_buf* using the initialisation value *nonce* and optionally
        authenticating the additional data in *adata*. If no *out_buf*
        is given the result is returned as a newly allocated `bytes`
        object. Otherwise, the result is written into mutable buffer
        *out_buf*.
        
