#include <hocon/parser/config_document_factory.hpp>
#include <hocon/config.hpp>

#include <iostream>
#include <set>

using namespace hocon;
using namespace std;

int main() {
    auto conf_file = config::parse_file_any_syntax("../conf/a.conf");
    auto conf = conf_file->resolve();
    auto conf2 = config::parse_file_any_syntax("../conf/sub/b.conf")->resolve();
    try {
        auto str = conf->get_string("Peter.passwd1");
        cout << "peter's passwd:" << str << endl;
        cout << "peter's passwd2:" << conf->get_string("Peter.passwd2") << endl;
        cout << "peter's passwd3:" << conf->get_string("Peter.passwd3") << endl;
        cout << "peter's passwd4:" << conf->get_string("Peter.passwd4") << endl;

        cout << "nick_name:" << conf2->get_string("other_field.nick_name") << endl;
    } catch (hocon::config_exception& e) {
        cout << "ERROR:" << e.what() << endl;
    }
    return 0;
}
