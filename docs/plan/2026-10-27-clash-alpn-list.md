# Clash export: split comma-separated ALPN into a list

## Rationale

Parsers for hysteria, hysteria2 and TUIC links store ALPN as a single
comma-joined string (`Proxy::Alpn`, e.g. `h3,h2,http/1.1`). The Clash
exporter pushed that string as one list item, producing
`alpn: ["h3,h2,http/1.1"]`. Mihomo expects one protocol per item
(https://wiki.metacubex.one/en/config/proxies/hysteria2), so the value was
unusable. The sing-box exporter already split on commas.

## Decisions

- Fix in the Clash exporter (`proxyToClash`), not in each parser: every
  `singleproxy["alpn"].push_back(x.Alpn)` now pushes each item of
  `split(x.Alpn, ",")`.
- Applied to all five call sites (VMess and Trojan fallbacks, Hysteria,
  Hysteria2, TUIC), since they share the same defect.
- `Proxy::Alpn` stays a joined string; `AlpnList` is unchanged.

## Impact

Clash/ClashR/mihomo output for these types now emits one ALPN per list item.
Single-value ALPN output is unchanged.

## Verification

Built `build/subconverter`, ran it, and requested
`/sub?target=clash&list=true&url=<hysteria2 link with alpn=h3%2Ch2%2Chttp%2F1.1>`.
Output: `alpn: [h3, h2, http/1.1]`. The sing-box output for the same link is
unchanged (`"alpn":["h3","h2","http/1.1"]`).
