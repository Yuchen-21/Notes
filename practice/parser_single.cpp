#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include <cassert>

static constexpr uint8_t HDR[4] = {0xAA, 0xAA, 0x55, 0x55};
static constexpr size_t  HDR_LEN = 4;
static constexpr size_t  LEN_LEN = 2;
static constexpr size_t  MAX_BODY = 4096;

static uint16_t read_be16(const uint8_t* p) {
    return (uint16_t(p[0]) << 8) | uint16_t(p[1]);
}

static void write_be16(uint8_t* p, uint16_t v) {
    p[0] = uint8_t(v >> 8);
    p[1] = uint8_t(v & 0xFF);
}

struct Frame {
    std::vector<uint8_t> body;
};

class ByteBuffer {
public:
    void append(const uint8_t* p, size_t n) {
        if (n == 0) return;
        ensure(w + n);
        std::memcpy(buf.data() + w, p, n);
        w += n;
    }

    size_t size() const { return w - r; }
    const uint8_t* data() const { return buf.data() + r; }

    void consume(size_t n) {
        if (n > size()) n = size();
        r += n;
        // 延迟整理：读指针走得太远再整理，避免频繁 memmove
        if (r > 0 && (r > buf.size() / 2)) compact();
    }

    void compact() {
        if (r == 0) return;
        size_t remain = size();
        if (remain > 0) std::memmove(buf.data(), buf.data() + r, remain);
        r = 0;
        w = remain;
    }

private:
    void ensure(size_t need) {
        if (need <= buf.size()) return;
        compact();
        if (need <= buf.size()) return;
        size_t cap = buf.empty() ? 1024 : buf.size();
        while (cap < need) cap *= 2;
        buf.resize(cap);
    }

    std::vector<uint8_t> buf;
    size_t r{0}, w{0};
};

// 单线程拆包器：从 ByteBuffer 中不断提取 Frame
class FrameParser {
public:
    explicit FrameParser(size_t max_body = MAX_BODY) : max_body_(max_body) {}

    void feed(const uint8_t* p, size_t n) { bb_.append(p, n); }

    // 成功提取一帧则返回 true
    bool try_pop(Frame& out) {
        // 解析策略：
        // 1) 缓冲区不足则返回 NeedMore
        // 2) 在缓冲区扫描 header，同步失败则保留 HDR_LEN-1 字节等待跨包
        // 3) header 对齐后读 len，做上限校验（防坏包/防内存炸）
        // 4) 数据不足则等更多数据，足够则提取 body 并 consume 整帧
        while (true) {
            // 1) 首先判断缓冲区是否足够包含 header+len
            if (bb_.size() < HDR_LEN + LEN_LEN) return false;

            // 2) 找 header
            size_t pos = find_header();
            if (pos == npos) {
                // 找不到：保留最多 3 字节（防止 header 跨包）
                size_t keep = std::min<size_t>(HDR_LEN-1, bb_.size());
                bb_.consume(bb_.size() - keep);
                return false;
            }

            // 3) 丢掉 header 前面的垃圾
            if (pos > 0) bb_.consume(pos);
            if (bb_.size() < HDR_LEN + LEN_LEN) return false;

            // 4) 解析 len
            const uint8_t* p0 = bb_.data();
            uint16_t mlen = read_be16(p0 + HDR_LEN);

            // 4.1) 长度异常：丢 1 字节继续重同步
            if (mlen > max_body_) {
                bb_.consume(1);
                continue;
            }

            size_t frame_len = HDR_LEN + LEN_LEN + size_t(mlen);
            if (bb_.size() < frame_len) return false; // 半包

            // 5) 取 body
            out.body.assign(p0 + HDR_LEN + LEN_LEN, p0 + frame_len);
            bb_.consume(frame_len);
            return true;
        }
    }

private:
    static constexpr size_t npos = size_t(-1);

    size_t find_header() const {
        const uint8_t* p = bb_.data();
        size_t n = bb_.size();
        if (n < HDR_LEN) return npos;
        for (size_t i = 0; i + HDR_LEN <= n; ++i) {
            if (p[i]==HDR[0] && p[i+1]==HDR[1] && p[i+2]==HDR[2] && p[i+3]==HDR[3]) {
                return i;
            }
        }
        return npos;
    }

    ByteBuffer bb_;
    size_t max_body_;
};

// 组帧（学习版）
static std::vector<uint8_t> make_frame_bytes(const std::string& body) {
    uint16_t len = (uint16_t)body.size();
    std::vector<uint8_t> v(HDR_LEN + LEN_LEN + len);
    v[0]=HDR[0]; v[1]=HDR[1]; v[2]=HDR[2]; v[3]=HDR[3];
    write_be16(v.data() + HDR_LEN, len);
    std::memcpy(v.data() + HDR_LEN + LEN_LEN, body.data(), len);
    return v;
}

static std::string to_string(const std::vector<uint8_t>& v) {
    return std::string(v.begin(), v.end());
}

// 把 parser 里能取出的帧都取出来
static std::vector<std::string> drain_all(FrameParser& parser) {
    std::vector<std::string> out;
    Frame fr;
    while (parser.try_pop(fr)) {
        out.emplace_back(fr.body.begin(), fr.body.end());
    }
    return out;
}
static void test_half_packet() {
    FrameParser parser;

    auto f1 = make_frame_bytes(R"({"msgType":1,"data":"hello"})");

    // 半包：拆成三段喂入
    parser.feed(f1.data(), 3);
    auto a = drain_all(parser);
    assert(a.empty()); // 不足一帧

    parser.feed(f1.data() + 3, 2);
    auto b = drain_all(parser);
    assert(b.empty()); // 仍不足一帧

    parser.feed(f1.data() + 5, f1.size() - 5);
    auto c = drain_all(parser);
    assert(c.size() == 1);
    assert(c[0] == R"({"msgType":1,"data":"hello"})");
}

static void test_sticky_packet() {
    FrameParser parser;

    auto f1 = make_frame_bytes(R"({"msgType":1,"data":"hello"})");
    auto f2 = make_frame_bytes(R"({"msgType":2,"data":"world"})");

    // 粘包：一次喂入两帧
    std::vector<uint8_t> stream;
    stream.insert(stream.end(), f1.begin(), f1.end());
    stream.insert(stream.end(), f2.begin(), f2.end());

    parser.feed(stream.data(), stream.size());
    auto out = drain_all(parser);

    assert(out.size() == 2);
    assert(out[0] == R"({"msgType":1,"data":"hello"})");
    assert(out[1] == R"({"msgType":2,"data":"world"})");
}

static void test_garbage_resync() {
    FrameParser parser;

    auto f1 = make_frame_bytes(R"({"msgType":1,"data":"hello"})");
    auto f2 = make_frame_bytes(R"({"msgType":2,"data":"world"})");

    std::vector<uint8_t> stream;
    // 1) 前面加垃圾
    stream.insert(stream.end(), {0x00, 0x11, 0x22, 0x33, 0x44});
    // 2) 加一帧
    stream.insert(stream.end(), f1.begin(), f1.end());
    // 3) 中间插入“坏包”片段：伪造 header + 超大 len（触发 BadLength 逻辑）
    {
        stream.insert(stream.end(), {0xAA,0xAA,0x55,0x55});
        // len = 0xFFFF (65535) > MAX_BODY => 应该被丢弃并继续 resync
        stream.insert(stream.end(), {0xFF,0xFF});
        // 注意：这里不放 body，让它更像异常/半包
        // parser 需要能从这段异常数据后恢复同步
    }
    // 4) 再加第二帧
    stream.insert(stream.end(), f2.begin(), f2.end());

    // 分几次喂入，模拟 recv 抖动
    size_t cut1 = 7;
    size_t cut2 = 13;
    parser.feed(stream.data(), cut1);
    (void)drain_all(parser);

    parser.feed(stream.data() + cut1, cut2 - cut1);
    (void)drain_all(parser);

    parser.feed(stream.data() + cut2, stream.size() - cut2);
    auto out = drain_all(parser);

    // 应该能解析出两帧（坏包被跳过）
    assert(out.size() == 2);
    assert(out[0] == R"({"msgType":1,"data":"hello"})");
    assert(out[1] == R"({"msgType":2,"data":"world"})");
}

int main() {
    test_half_packet();
    test_sticky_packet();
    test_garbage_resync();
    std::cout << "ALL TESTS PASSED\n";
    return 0;
}
// int main() {
//     FrameParser parser;

//     auto f1 = make_frame_bytes(R"({"msgType":1,"data":"hello"})");
//     auto f2 = make_frame_bytes(R"({"msgType":2,"data":"world"})");

//     // 构造测试流：垃圾 + 半包 + 粘包
//     std::vector<uint8_t> stream;
//     stream.insert(stream.end(), {0x00,0x11,0x22,0x33});        // garbage
//     stream.insert(stream.end(), f1.begin(), f1.begin()+5);      // half
//     stream.insert(stream.end(), f1.begin()+5, f1.end());        // rest
//     stream.insert(stream.end(), f2.begin(), f2.end());          // stick

//     // 分多次喂入（模拟 recv）
//     parser.feed(stream.data(), 6);
//     parser.feed(stream.data()+6, 10);
//     parser.feed(stream.data()+16, stream.size()-16);

//     Frame fr;
//     int cnt = 0;
//     while (parser.try_pop(fr)) {
//         std::cout << "Frame[" << cnt++ << "] body=" << to_string(fr.body) << "\n";
//     }

//     // 简单断言：应该解析出 2 帧
//     assert(cnt == 2);
//     std::cout << "OK, extracted=" << cnt << "\n";
//     return 0;
// }