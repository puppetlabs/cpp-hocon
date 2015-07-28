#include <internal/config_number.hpp>
#include <internal/simple_config_origin.hpp>

#include <string>

namespace hocon {

    class config_double : public config_number {
    public:
        config_double(simple_config_origin origin, double value, std::string original_text);

        std::string transform_to_string() override;

        long long_value() const;
        double double_value() const;

        config_double* new_copy(simple_config_origin origin);


    private:
        double _value;
    };

}  // namespace hocon
