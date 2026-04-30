#include <iostream>
#include <string>
#include <vector>

#include "rapidjson/document.h"
#include "generator/config/nodemanip.h"
#include "generator/config/subexport.h"
#include "handler/settings.h"
#include "parser/subparser.h"

void explodeClash(YAML::Node yamlnode, std::vector<Proxy> &nodes);

Settings global;

int addNodes(std::string, std::vector<Proxy> &, int, parse_settings &) {
    return 0;
}

int main() {
    Proxy node;
    explodeStdVless("vless://cbe9458d-93ec-4e47-afd4-6817dfd60d8d@3uprc8.tencentapp.cn:443?security=tls&type=xhttp&path=/zones&host=vrfhzw.m3u8.dsicbuyga.pay.uruu.cn&sni=vrfhzw.m3u8.dsicbuyga.pay.uruu.cn&flow=xtls-rprx-vision&fp=firefox&alpn=h2&encryption=mlkem#xhttp-test", node);

    if (node.Type != ProxyType::VLESS) {
        std::cerr << "expected VLESS node\n";
        return 1;
    }
    if (node.TransferProtocol != "xhttp") {
        std::cerr << "expected xhttp transport, got " << node.TransferProtocol << "\n";
        return 1;
    }
    if (node.Path != "/zones") {
        std::cerr << "expected /zones path, got " << node.Path << "\n";
        return 1;
    }
    if (node.Host != "vrfhzw.m3u8.dsicbuyga.pay.uruu.cn") {
        std::cerr << "expected xhttp host, got " << node.Host << "\n";
        return 1;
    }

    std::vector<Proxy> singboxNodes{node};
    std::vector<RulesetContent> rulesets;
    ProxyGroupConfigs singboxGroups;
    extra_settings singboxExt;
    singboxExt.nodelist = true;
    std::string singbox = proxyToSingBox(singboxNodes, "{}", rulesets, singboxGroups, singboxExt);
    rapidjson::Document singboxJson;
    singboxJson.Parse(singbox.c_str());
    if (singboxJson.HasParseError() || !singboxJson.HasMember("outbounds") || !singboxJson["outbounds"].IsArray() ||
        singboxJson["outbounds"].Size() != 1) {
        std::cerr << "expected one sing-box outbound for xhttp VLESS, got " << singbox << "\n";
        return 1;
    }
    const auto &singboxProxy = singboxJson["outbounds"][0];
    if (!singboxProxy.HasMember("transport") || !singboxProxy["transport"].IsObject()) {
        std::cerr << "expected sing-box xhttp transport, got " << singbox << "\n";
        return 1;
    }
    const auto &singboxTransport = singboxProxy["transport"];
    if (!singboxTransport.HasMember("type") || std::string(singboxTransport["type"].GetString()) != "xhttp" ||
        !singboxTransport.HasMember("path") || std::string(singboxTransport["path"].GetString()) != "/zones") {
        std::cerr << "expected sing-box xhttp transport type and path, got " << singbox << "\n";
        return 1;
    }

    YAML::Node input = YAML::Load(R"(
proxies:
  - name: xhttp-test
    type: vless
    server: 3uprc8.tencentapp.cn
    port: 443
    uuid: cbe9458d-93ec-4e47-afd4-6817dfd60d8d
    udp: true
    tls: true
    skip-cert-verify: false
    client-fingerprint: firefox
    servername: vrfhzw.m3u8.dsicbuyga.pay.uruu.cn
    flow: xtls-rprx-vision
    network: xhttp
    alpn:
      - h2
    xhttp-opts:
      path: /zones
      mode: stream-up
      x-padding-bytes: 100-1000
      no-grpc-header: false
      no-sse-header: false
      sc-max-each-post-bytes: '1000000'
      sc-min-posts-interval-ms: '5'
      sc-stream-up-server-secs: 5-10
      xmux:
        max-concurrency: '1'
        max-connections: '0'
        c-max-reuse-times: '0'
        h-max-request-times: 3000-5000
        h-max-reusable-secs: 7776000-10368000
        h-keep-alive-period: 0
      host: vrfhzw.m3u8.dsicbuyga.pay.uruu.cn
    encryption: mlkem
)");

    std::vector<Proxy> nodes;
    explodeClash(input, nodes);
    if (nodes.size() != 1) {
        std::cerr << "expected one parsed clash node, got " << nodes.size() << "\n";
        return 1;
    }
    if (nodes[0].TransferProtocol != "xhttp") {
        std::cerr << "expected parsed clash xhttp transport, got " << nodes[0].TransferProtocol << "\n";
        return 1;
    }
    if (nodes[0].XHTTP.Mode != "stream-up" || nodes[0].XHTTP.XmuxMaxConcurrency != "1") {
        std::cerr << "expected parsed xhttp options to be preserved\n";
        return 1;
    }

    extra_settings ext;
    ext.nodelist = true;
    ext.clash_new_field_name = true;
    YAML::Node output;
    ProxyGroupConfigs groups;
    proxyToClash(nodes, output, groups, false, ext);
    YAML::Node outProxy = output["proxies"][0];
    if (outProxy["network"].as<std::string>() != "xhttp") {
        std::cerr << "expected exported xhttp network, got " << outProxy["network"].as<std::string>() << "\n";
        return 1;
    }
    if (!outProxy["xhttp-opts"].IsDefined() || outProxy["h2-opts"].IsDefined()) {
        std::cerr << "expected xhttp-opts without h2-opts\n";
        return 1;
    }
    if (outProxy["xhttp-opts"]["mode"].as<std::string>() != "stream-up" ||
        outProxy["xhttp-opts"]["xmux"]["max-concurrency"].as<std::string>() != "1") {
        std::cerr << "expected exported xhttp options to be preserved\n";
        return 1;
    }
    return 0;
}
