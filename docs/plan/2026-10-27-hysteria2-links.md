# Hysteria2 links: port hopping parsing and `obfs-password` export

## Parsing `mport` as port-hopping range

### Rationale

Several clients (e.g. v2rayN) share hysteria2 links with the port-hopping
range in `mport` (`?mport=40000-50000`). `explodeStdHysteria2` only read
`ports`, so the range was dropped and Clash output had no `ports` field.

### Decisions

- Read `ports` first; fall back to `mport` when `ports` is absent.
- URL-decode both values, so encoded multi-range values such as
  `1000%2C2000-3000` become `1000,2000-3000`.

### Impact

Hysteria2 links carrying `mport` now produce `ports: <range>` in Clash output.
Links with `ports` behave as before, except that an encoded value is now
decoded.

### Verification

Rebuilt `build/subconverter` and requested `/sub?target=clash&list=true`:

| Link query | Clash output |
|---|---|
| `mport=40000-50000` | `ports: 40000-50000` |
| `mport=1000%2C2000-3000` | `ports: "1000,2000-3000"` |
| `ports=5000-6000&mport=40000-50000` | `ports: 5000-6000` |
| neither | no `ports` field |

## Exporting `obfs-password`

### Rationale

`proxyToSingle` built hysteria2 links with `obfs-password=<obfs type>`
(`obfsparam`) instead of the password, so an exported link carried
`obfs-password=salamander` and could not connect.

### Decisions

- Use `obfsPassword` (`Proxy::OBFSPassword`) for the value. Encoding is left
  as-is to match the other query parameters in the same link.

### Impact

Hysteria2 links from `target=mixed` (and other single-link targets) now carry
the real obfs password.

### Verification

Rebuilt and requested `/sub?target=mixed` for a link with
`obfs=salamander&obfs-password=example-pw`. Decoded output contains
`obfs=salamander&obfs-password=example-pw`.

## Parsing multi-port authority

### Rationale

The Hysteria2 URI format puts port hopping in the authority
(`host:443,5000-6000`), and `proxyToSingle` exports links that way.
`explodeStdHysteria2` only accepted `host:<digits>`, so such links (including
its own exports) were dropped.

### Decisions

- The authority port accepts `[\d,\-]+`. When it holds `,` or `-`, the first
  number becomes `port` and the full spec becomes `ports`.
- Query `ports`/`mport` take precedence over the authority spec.

### Impact

Links like `hysteria2://pw@host:443,5000-6000` now parse, and Clash output gets
`port: 443` plus `ports: "443,5000-6000"` (mihomo ignores `port` when `ports`
is set). Single-port links are unchanged.

### Verification

Rebuilt and requested `/sub?target=clash&list=true`:

| Link | Clash output |
|---|---|
| `pw@example.com:24101,40000-50000` | `port: 24101`, `ports: "24101,40000-50000"` |
| `pw@example.com:5000-6000` | `port: 5000`, `ports: 5000-6000` |
| `pw@example.com:443,5000-6000?ports=7000-8000` | `ports: 7000-8000` |
| `pw@[2001:db8::1]:443,5000-6000` | `server: 2001:db8::1`, `ports: "443,5000-6000"` |
| `example.com:443,5000-6000?password=pw` | `ports: "443,5000-6000"` |
| `pw@example.com:443` | no `ports` field |

Round trip: a link with `mport=40000-50000` exported via `target=mixed` as
`example.com:24101,40000-50000`, and re-importing it gave the same `ports`.
