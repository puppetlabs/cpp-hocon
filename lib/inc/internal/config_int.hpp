#include <internal/config_number.hpp>
#include <internal/simple_config_origin.hpp>

#include <string>

namespace hocon {

    class config_int : public config_number {
    public:
        config_int(simple_config_origin origin, int value, std::string original_text);

        std::string transform_to_string() const override;

        long long_value() const;
        double double_value() const;

    private:
        int _value;
    };

}  // namespace hocon
