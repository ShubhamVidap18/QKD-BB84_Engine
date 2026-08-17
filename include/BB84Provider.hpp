#pragma once

#include "BB84Config.hpp"
#include "IQKDProvider.hpp"

#include <atomic>
#include <mutex>
#include <optional>

namespace bb84 {

class BB84Provider final : public IQKDProvider {
public:
    explicit BB84Provider(BB84Config config = {});
    ~BB84Provider() override;

    void initialize() override;
    void openConnection() override;
    ByteVector getKey(const KeyId& key_id) override;
    void closeConnection() override;

    [[nodiscard]] ProviderStatus getStatus() const noexcept override;
    [[nodiscard]] double getQBER() const noexcept override;
    [[nodiscard]] double getKeyRate() const noexcept override;

    void shutdown() noexcept override;

private:
    // Internal orchestration entry point. It is intentionally not part of
    // IQKDProvider's public API.
    ByteVector generateKeyInternal(const KeyId& key_id);

    bool transitionLocked(ProviderStatus expected, ProviderStatus next) noexcept;
    void requireStateLocked(ProviderStatus expected) const;

    BB84Config config_;

    mutable std::mutex mutex_;
    ProviderStatus status_{ProviderStatus::UNINITIALIZED};

    double qber_{0.0};
    double key_rate_{0.0};

    std::optional<KeyId> active_key_id_;
    ByteVector active_key_;
};

}
