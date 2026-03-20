#include "channel_model.h"
#include "harq.h"
#include "proportional_fair_scheduler.h"
#include "round_robin_scheduler.h"
#include "ue.h"

#include <gtest/gtest.h>
#include <map>
#include <set>

using namespace telecom;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static std::deque<UE> makeQueue(uint32_t count, double baseCqi = 5.0,
                                size_t packets = 2) {
    std::deque<UE> q;
    for (uint32_t id = 1; id <= count; ++id) {
        UE ue;
        ue.id              = id;
        ue.lastReportedCqi = baseCqi + id;
        ue.pendingPackets  = std::deque<size_t>(packets, 1024);
        q.push_back(std::move(ue));
    }
    return q;
}

// ---------------------------------------------------------------------------
// RoundRobinScheduler
// ---------------------------------------------------------------------------
TEST(RoundRobinScheduler, AllocatesAtLeastOne) {
    auto q = makeQueue(4);
    RoundRobinScheduler rr;
    auto alloc = rr.schedule(q, 2);
    EXPECT_FALSE(alloc.empty());
    EXPECT_LE(alloc.size(), 2u);
}

TEST(RoundRobinScheduler, NeverExceedsAvailableRbs) {
    auto q = makeQueue(10);
    RoundRobinScheduler rr;
    auto alloc = rr.schedule(q, 3);
    EXPECT_LE(alloc.size(), 3u);
}

TEST(RoundRobinScheduler, RotatesAcrossTtis) {
    // With 4 UEs and 2 RBs per TTI the first two TTIs should together cover
    // all 4 UEs (no UE gets starved).
    auto q = makeQueue(4);
    RoundRobinScheduler rr;
    auto a1 = rr.schedule(q, 2);
    auto a2 = rr.schedule(q, 2);
    std::set<uint32_t> seen(a1.begin(), a1.end());
    seen.insert(a2.begin(), a2.end());
    EXPECT_EQ(seen.size(), 4u);
}

TEST(RoundRobinScheduler, SkipsUesWithNoData) {
    auto q = makeQueue(4);
    q[1].pendingPackets.clear(); // UE 2 has no data
    RoundRobinScheduler rr;
    auto alloc = rr.schedule(q, 4);
    for (auto id : alloc) {
        EXPECT_NE(id, 2u);
    }
}

TEST(RoundRobinScheduler, EmptyQueueReturnsEmpty) {
    std::deque<UE> empty;
    RoundRobinScheduler rr;
    EXPECT_TRUE(rr.schedule(empty, 5).empty());
}

TEST(RoundRobinScheduler, ZeroRbsReturnsEmpty) {
    auto q = makeQueue(3);
    RoundRobinScheduler rr;
    EXPECT_TRUE(rr.schedule(q, 0).empty());
}

// ---------------------------------------------------------------------------
// ProportionalFairScheduler
// ---------------------------------------------------------------------------
TEST(ProportionalFairScheduler, AllocatesAtLeastOne) {
    auto q = makeQueue(4);
    ProportionalFairScheduler pf;
    auto alloc = pf.schedule(q, 2);
    EXPECT_FALSE(alloc.empty());
    EXPECT_LE(alloc.size(), 2u);
}

TEST(ProportionalFairScheduler, PrefersHighCqiUeInitially) {
    // On the very first TTI all average throughputs are equal (1e-6),
    // so the UE with the highest CQI should be chosen.
    auto q = makeQueue(4); // CQI: UE1=6, UE2=7, UE3=8, UE4=9
    ProportionalFairScheduler pf;
    auto alloc = pf.schedule(q, 1);
    ASSERT_EQ(alloc.size(), 1u);
    EXPECT_EQ(alloc[0], 4u); // UE4 has the highest CQI
}

TEST(ProportionalFairScheduler, EvolvesAverageThroughput) {
    // Run many TTIs; a UE that is always scheduled should eventually see its
    // average throughput rise and be deprioritised in favour of others.
    auto q = makeQueue(4);
    ProportionalFairScheduler pf;
    std::map<uint32_t, int> counts;
    for (int i = 0; i < 50; ++i) {
        auto alloc = pf.schedule(q, 1);
        for (auto id : alloc) counts[id]++;
    }
    // All UEs should receive at least one allocation over 50 TTIs.
    for (uint32_t id = 1; id <= 4; ++id) {
        EXPECT_GT(counts[id], 0) << "UE " << id << " was never scheduled";
    }
}

TEST(ProportionalFairScheduler, EmptyQueueReturnsEmpty) {
    std::deque<UE> empty;
    ProportionalFairScheduler pf;
    EXPECT_TRUE(pf.schedule(empty, 5).empty());
}

// ---------------------------------------------------------------------------
// ChannelModel
// ---------------------------------------------------------------------------
TEST(ChannelModel, SnrToCqiMonotone) {
    uint8_t prev = ChannelModel::snrToCqi(-10.0);
    for (double snr = -9.0; snr <= 35.0; snr += 1.0) {
        uint8_t cur = ChannelModel::snrToCqi(snr);
        EXPECT_GE(cur, prev) << "CQI decreased at SNR=" << snr;
        prev = cur;
    }
}

TEST(ChannelModel, SnrToCqiRange) {
    for (double snr = -20.0; snr <= 40.0; snr += 2.5) {
        uint8_t cqi = ChannelModel::snrToCqi(snr);
        EXPECT_GE(cqi, 0);
        EXPECT_LE(cqi, 15);
    }
}

TEST(ChannelModel, CqiToMcsRange) {
    for (uint8_t cqi = 0; cqi <= 15; ++cqi) {
        uint8_t mcs = ChannelModel::cqiToMcs(cqi);
        EXPECT_LE(mcs, 15);
    }
}

// ---------------------------------------------------------------------------
// HarqManager
// ---------------------------------------------------------------------------
TEST(HarqManager, AllowsUpToMaxRetransmissions) {
    HarqManager mgr;
    HarqProcess proc{1, 0, false};
    for (uint32_t i = 0; i < 4; ++i) {
        EXPECT_TRUE(mgr.requestRetransmission(proc));
    }
    EXPECT_FALSE(mgr.requestRetransmission(proc)); // 5th attempt should fail
}

TEST(HarqManager, ResetClearsState) {
    HarqManager mgr;
    HarqProcess proc{1, 0, false};
    mgr.requestRetransmission(proc);
    mgr.requestRetransmission(proc);
    mgr.reset(proc);
    EXPECT_EQ(proc.retransmissionCount, 0u);
    EXPECT_FALSE(proc.acked);
    // Should be able to retransmit again after reset.
    EXPECT_TRUE(mgr.requestRetransmission(proc));
}
