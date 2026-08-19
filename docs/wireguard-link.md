# WireGuard link format

subconverter accepts WireGuard nodes as a single-line URI, next to `ss://`,
`vmess://`, `trojan://` and friends. Both `wireguard://` and the Shadowrocket
style `wg://` prefix are recognised.

## Syntax

```
wireguard://<private-key>@<server>:<port>?<params>#<remark>
```

| Part          | Required | Notes                                                                 |
|---------------|----------|-----------------------------------------------------------------------|
| `private-key` | yes*     | Local interface private key, URL-encoded (base64 contains `+/=`). May be given as the `privatekey` parameter instead. |
| `server`      | yes      | Peer endpoint host or IP. IPv6 must be bracketed: `[2001:db8::1]`.     |
| `port`        | no       | Peer endpoint port, defaults to `51820`.                               |
| `remark`      | no       | Node name, URL-encoded. Defaults to `server:port`.                     |

## Parameters

| Parameter | Alias | Meaning |
|-----------|-------|---------|
| `address` | `ip` | Comma separated interface addresses. A CIDR suffix is accepted and stripped (`10.0.0.2/32` → `10.0.0.2`). The first IPv4 becomes the v4 address, the first IPv6 the v6 address. |
| `publickey` | `public_key` | Peer public key. |
| `privatekey` | — | Local private key, when it is not placed before `@`. |
| `presharedkey` | `pre_shared_key` | Peer pre-shared key. |
| `mtu` | — | Interface MTU, e.g. `1420`. |
| `keepalive` | `persistent_keepalive` | Persistent keepalive in seconds. |
| `reserved` | — | Comma separated reserved bytes / client id, e.g. `1,2,3` (used by WARP). |
| `dns` | — | Comma separated DNS servers. |

All values may be URL-encoded; keys almost always need to be, because base64
padding (`=`) and `+` are not safe in a query string.

## Examples

Minimal:

```
wireguard://wIvL...%3D@192.0.2.1:51820?address=10.0.0.2/32&publickey=Tm9k...%3D#MyWG
```

Full:

```
wireguard://wIvL...%3D@192.0.2.1:51820?address=10.0.0.2/32,fd01::2/128&publickey=Tm9k...%3D&presharedkey=cHNr&mtu=1420&keepalive=25&reserved=1,2,3&dns=1.1.1.1,8.8.8.8#MyWG
```

Usage is the same as any other link:

```
http://127.0.0.1:25500/sub?target=clash&url=<url-encoded link>
```

Several links can be joined with `|`, or supplied base64-encoded as a
subscription body.

## How fields map to each target

| Field | Clash / mihomo | Surge (>= 4) | Quantumult X | sing-box |
|-------|----------------|--------------|--------------|----------|
| address v4 | `ip` | `self-ip` | `interface-ip` | `address` (as `/32`) |
| address v6 | `ipv6` | `self-ip-v6` | `interface-ipv6` | `address` (as `/128`) |
| private key | `private-key` | `private-key` | `private-key` | `private_key` |
| public key | `public-key` | peer `public-key` | peer `public-key` | peer `public_key` |
| preshared key | `pre-shared-key` | `preshared-key` | — | peer `pre_shared_key` |
| dns | `dns` + `remote-dns-resolve` | `dns-server` | `dns` / `dnsv6` | — |
| mtu | `mtu` | `mtu` | `mtu` | `mtu` |
| keepalive | `persistent-keepalive` | `keepalive` | `keepalive` | peer `persistent_keepalive_interval` |
| reserved | `reserved` | peer `client-id` / `reserved` | peer `reserved` | peer `reserved` |
| allowed ips | `allowed-ips` | peer `allowed-ips` | — | peer `allowed_ips` |

Notes:

- Surge emits WireGuard only for Surge 4 and above; older targets skip the node.
- `allowed-ips` is not part of the link format; it defaults to `0.0.0.0/0, ::/0`.
- sing-box removed the WireGuard outbound in 1.13, so nodes are emitted into the
  top-level `endpoints` array instead of `outbounds`. Proxy groups reference them
  by tag exactly like an outbound.
- mihomo only uses `dns` when `remote-dns-resolve` is true, so it is set whenever
  the link carries DNS servers.
