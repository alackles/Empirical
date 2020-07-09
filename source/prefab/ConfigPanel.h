#ifndef EMP_CONFIG_WEB_INTERFACE_H
#define EMP_CONFIG_WEB_INTERFACE_H

#include "../config/config.h"
#include "../web/Div.h"
#include "../web/Element.h"
#include "../web/Input.h"

#include <map>
#include <set>
#include "../tools/set_utils.h"
#include "../tools/string_utils.h"

// Prefab elements
#include "Card.h"
#include "CommentBox.h"
#include "FontAwesomeIcon.h"
#include "Collapse.h"
#include "ToggleSwitch.h"

namespace emp {
    namespace prefab{
    class ConfigPanel {
        private:
            inline static std::set<std::string> numeric_types = {"int", "double", "float", "uint32_t", "uint64_t", "size_t"};
            Config & config;
            web::Div settings_div;
            std::set<std::string> exclude;
            std::map<std::string, web::Div> group_divs;
            std::map<std::string, web::Div> input_divs;
            std::function<void(const std::string & val)> on_change_fun = [](const std::string & val){;};
            std::function<std::string(std::string val)> format_label_fun = [](std::string name){
                emp::vector<std::string> sliced = slice(name, '_');
                return to_titlecase(join(sliced, " "));
            };
            void SyncForm(std::string val, std::string input1, std::string input2){
                emp::web::Input div1(settings_div.Find(input1));
                div1.Value(val);
                emp::web::Input div2(settings_div.Find(input2));
                div2.Value(val);
                div1.Redraw();
                div2.Redraw();
            }
        public:
            ConfigPanel(Config & c, const std::string & div_name = "settings_div") 
                : config(c), settings_div(div_name) {;}

            void SetOnChangeFun(std::function<void(const std::string & val)> fun) {on_change_fun = fun;}
            
            template <typename T>
            void SetDefaultRangeFloatingPoint(web::Input & input, T val) {
                if (val > 0 && val < 1) {
                    // This is a common range for numbers to be in
                    input.Min(0);   
                    if (val > .1) {
                        input.Max(1);
                    } else {
                        input.Max(val * 100);
                    }
                    input.Step(val/10.0);      
                } else if (val > 0) {
                    // Assume this is a positive number
                    input.Min(0);
                    input.Max(val * 10);
                    input.Step(val/10.0);
                } else if (val < 0) {
                    input.Min(val * 10); // since val is negative
                    input.Max(val * -10);
                    input.Step(val/-10.0); // A negative step would be confusing
                }

                // Otherwise val is 0 and we have nothing to go on
            }

            void SetDefaultRangeFixedPoint(web::Input & input, int val) {
                // Default step is 1, which should be fine for fixed point

                if (val > 0) {
                    // Assume this is a positive number
                    input.Min(0);
                    input.Max(val * 10);
                } else if (val < 0) {
                    input.Min(val * 10); // since val is negative
                    input.Max(val * -10);
                }

                // Otherwise val is 0 and we have nothing to go on                
            }

            void ExcludeConfig(std::string setting) {
                exclude.insert(setting);
            } 

            void Setup(const std::string & id_prefix = "settings_") {
                for (auto group : config.GetGroupSet()) {
                    // std::cout << "GROUP: " << group->GetName() << std::endl;
                    std::string group_name = group->GetName();
                    group_divs[group_name] = web::Div(id_prefix + group_name);
                    settings_div << group_divs[group_name]; 

                    // Prefab Card
                    prefab::Card card(prefab::Card::Collapse::OPEN);
                    group_divs[group_name] << card;

                    // Header content
                    web::Div setting_heading;
                    card.AddHeaderContent(setting_heading);
                    setting_heading << "<h3>" + group->GetDesc() + "</h3>";
                    setting_heading.SetAttr("class", "setting_heading");

                    for (size_t i = 0; i < group->GetSize(); i++) {
                        // std::cout << group->GetEntry(i)->GetType() << std::endl;
                        std::string name = group->GetEntry(i)->GetName();
                        if (Has(exclude, name)) {
                            continue;
                        }
                        std::string type = group->GetEntry(i)->GetType();
                        std::string value = group->GetEntry(i)->GetValue();
                   
                        card.AddBodyContent(input_divs[name]);

                        // Setting element label
                        web::Div setting_element(name + "_row");
                        input_divs[name] << setting_element;
                        setting_element.SetAttr("class", "setting_element");
                        web::Element title_span("span");
                        setting_element << title_span;
                        web::Element title("button");
                        title.SetAttr("class", "btn btn-link");

                        prefab::FontAwesomeIcon arrow_right_for_dropdown("fa-angle-double-right");
                        title << arrow_right_for_dropdown;
                        prefab::FontAwesomeIcon arrow_up_for_dropdown("fa-angle-double-up");
                        title << arrow_up_for_dropdown;
                        title << format_label_fun(name);
                        title_span.SetAttr("class", "title_area");
                        arrow_right_for_dropdown.AddClass("toggle_icon_right_margin");
                        arrow_up_for_dropdown.AddClass("toggle_icon_right_margin");
                        
                        // Prefab Dropdown Box 
                        prefab::CommentBox box;
                        box.AddContent(group->GetEntry(i)->GetDescription());

                        // Prefab Collapse/toggle for setting element
                        prefab::Collapse title_toggle(title, box, false, name + "_dropdown");
                        input_divs[name] << title_toggle.GetToggleDiv();
                        title_span << title_toggle.GetLinkDiv();


                        if (Has(numeric_types, type)) {
                            const std::string name_input_slider = name + "_input_slider";
                            const std::string name_input_number = name + "_input_number";
                            const std::string name_input_mobile_slider = name + "_input_mobile_slider";
                            web::Input slider( [](std::string x){
                                std::cout << "empty slider function" << std::endl;},
                            "range", NULL, name_input_slider
                            );
                            setting_element << slider;

                            web::Input number([](std::string val){
                                std::cout << "empty number function" << std::endl;
                                }, 
                                "number", NULL, name_input_number
                                );
                            setting_element << number;
                            web::Input mobile_slider([](std::string val){
                                std::cout << "empty mobile slider function" << std::endl;
                            }, 
                                "range", NULL, name_input_mobile_slider
                                ); 
                            box.AddMobileContent("<hr>");
                            box.AddMobileContent(mobile_slider);

                            // Set onchange behavior for inputs
                            slider.Callback(
                                [this,name, name_input_number, name_input_mobile_slider](std::string val){ 
                                config.Set(name, val);
                                SyncForm(val, name_input_number, name_input_mobile_slider);
                                });
                            number.Callback(
                                [this,name, name_input_slider, name_input_mobile_slider](std::string val){ 
                                config.Set(name, val);
                                SyncForm(val, name_input_slider, name_input_mobile_slider);
                                });
                            mobile_slider.Callback(
                                [this,name, name_input_number, name_input_slider](std::string val){ 
                                config.Set(name, val);
                                SyncForm(val, name_input_number, name_input_slider);
                                });
                            // Set initial values
                            slider.Value(config.Get(name));
                            number.Value(config.Get(name));
                            mobile_slider.Value(config.Get(name));
                            slider.SetAttr("class", "input_slider");
                            number.SetAttr("class", "input_number");

                            // Attempt to have intelligent defaults
                            if (type == "double") {
                                SetDefaultRangeFloatingPoint(slider, emp::from_string<double>(value));
                                SetDefaultRangeFloatingPoint(number, emp::from_string<double>(value));
                                SetDefaultRangeFloatingPoint(mobile_slider, emp::from_string<double>(value));
                            } else if (type == "float") {
                                SetDefaultRangeFloatingPoint(slider, emp::from_string<float>(value));
                                SetDefaultRangeFloatingPoint(number, emp::from_string<float>(value));
                                SetDefaultRangeFloatingPoint(mobile_slider, emp::from_string<float>(value));
                            } else {
                                // TODO: Correctly handle all types (although I'm not sure it actually matters?)
                                SetDefaultRangeFixedPoint(slider, emp::from_string<int>(value));
                                SetDefaultRangeFixedPoint(number, emp::from_string<int>(value));
                                SetDefaultRangeFixedPoint(mobile_slider, emp::from_string<int>(value));
                            }

                        } 
                        else if (type == "bool") {
                            // Bootstrap Toggle Switch (need at least v4.5.0)
                            emp::prefab::ToggleSwitch toggle_switch([this, name](std::string val){config.Set(name, val);
                                                              on_change_fun(val);},
                                NULL, emp::from_string<bool>(value), name + "_input_checkbox");
                            setting_element << toggle_switch;
                            toggle_switch.AddClass("input_bool");

                        } else {
                            web::Input text_input(
                                [this, name](std::string val){config.Set(name, val);
                                                               on_change_fun(val);},
                                "text", NULL, name + "_input_textbox"
                            );
                            setting_element << text_input;
                            text_input.SetAttr(
                                "class", "input_text",
                                "type", "text"
                            );
                            text_input.Value(config.Get(name));
                        }
                    }
                }

            }

            web::Div & GetDiv() {return settings_div;}

    };
    }
}

#endif
