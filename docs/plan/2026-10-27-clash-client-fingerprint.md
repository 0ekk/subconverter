# Clash export: `fp` to `client-fingerprint` for Trojan and AnyTLS

## Rationale

mihomo takes the uTLS fingerprint as `client-fingerprint` and applies it only
to VMess/VLESS/Trojan/AnyTLS
(https://wiki.metacubex.one/en/config/proxies/tls/). Only VLESS exported it.
Trojan links dropped `fp=chrome`, and AnyTLS emitted it as `fingerprint: chrome`,
which is the certificate-pin field.

## Decisions

- `Proxy::Fingerprint` holds either a uTLS name (`fp`) or a certificate pin
  (`hpkp`, Clash/Surge `fingerprint`), depending on the parser. The export
  classifies by value with `isUtlsFingerprint()` (anonymous namespace in
  `subexport.cpp`), reusing the uTLS name list that the sing-box VLESS export
  already had.
- Trojan: a known uTLS name becomes `client-fingerprint`; other values are still
  not exported, so an unknown value cannot fail mihomo's certificate-pin check.
- AnyTLS: a known uTLS name becomes `client-fingerprint`; any other value stays
  `fingerprint` as before.
- Hysteria2 is left out: it runs on QUIC, and mihomo ignores
  `client-fingerprint` there.
- Matching is exact (lowercase), as in the existing sing-box check.

## Impact

Clash output for Trojan/AnyTLS now carries the uTLS fingerprint. AnyTLS
certificate pins and the VLESS output are unchanged.

## Verification

Rebuilt and requested `/sub?target=clash&list=true`:

| Link | Clash output |
|---|---|
| `trojan://...&fp=chrome` | `client-fingerprint: chrome` |
| `trojan://...` (no fp) / `&fp=weird` | no fingerprint field |
| `anytls://...&fp=chrome` | `client-fingerprint: chrome` |
| `anytls://...&hpkp=ab12cd34` | `fingerprint: ab12cd34` |
| `vless://...&fp=firefox` | `client-fingerprint: firefox` (unchanged) |

The sing-box output for the VLESS link still has
`"utls":{"enabled":true,"fingerprint":"firefox"}`.
