from Crypto.Cipher import AES

PRIVATE_KEY = b'\x11\x22\x33\x44\x55\x66\x77\x88\x99\xAA\xBB\xCC\xDD\xEE\xFF\x00'
TOKEN = b'\x12\x34\x56\x78\x9A\xBC\xDE\xF0\x11\x22\x33\x44\x55\x66\x77\x88'

def bytesToStr(bytestr):
    return ', '.join([hex(ord(byte)) for byte in bytestr])

def signToken(token, key):
    assert(len(token) == 16)
    assert(len(key) == 16)
    aes = AES.new(key)
    return aes.encrypt(token)

if __name__ == '__main__':
    print("Private Key: " + bytesToStr(PRIVATE_KEY))
    print("Token: " + bytesToStr(TOKEN))
    signed_token = signToken(TOKEN, PRIVATE_KEY)
    print("Signed Token: " + bytesToStr(signed_token))
