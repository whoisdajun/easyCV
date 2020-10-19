//
// Created by li on 2020/10/19.
//

#ifndef EASYCV_TEMPLATE_UTILS_H
#define EASYCV_TEMPLATE_UTILS_H
namespace easyCV
{

// Used to prevent automatic template argument deduction
    template <typename T>
    struct dependent_type
    {
        using type = T;
    };

}

#endif //EASYCV_TEMPLATE_UTILS_H
