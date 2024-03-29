#pragma once
#include "../structs.h"
#include "vec3.h"
#include <algorithm>
#include <cstdlib>
#include <variant>
#include <optional>

struct boxnode
{
    struct branches{
        boxnode *left;
        boxnode *right;
    };
    b_b box;
    std::variant<branches, triangle> next;
};

using Tree = boxnode *;

struct TriangleRange
{
    triangle *begin;
    triangle *end;
};

//min max for float

float min_of_1(float x, float y, float z){
    return std::min(x,std::min(y,z));
};

float max_of_1(float x, float y, float z){
    return std::max(x,std::max(y,z));
};

//min max for vec3

vec3 min_of_3(vec3 a, vec3 b, vec3 c){
    return {
        min_of_1(a.x, b.x, c.x),
        min_of_1(a.y, b.y, c.y),
        min_of_1(a.z, b.z, c.z),
        };
}

vec3 max_of_3(vec3 a, vec3 b, vec3 c){
    return {
        max_of_1(a.x, b.x, c.x),
        max_of_1(a.y, b.y, c.y),
        max_of_1(a.z, b.z, c.z),
        };
}

//triangles to b_b

b_b triangle_to_b_b(triangle tr){
    vec3 min = min_of_3(tr.a, tr.b, tr.c);
    vec3 max = max_of_3(tr.a, tr.b, tr.c);
    return {
        min,
        max,
    };
};

vec3 min_of_2vectors(vec3 a, vec3 b){
    return{
        std::min(a.x, b.x),
        std::min(a.y, b.y),
        std::min(a.z, b.z),
    };
}

vec3 max_of_2vectors(vec3 a, vec3 b){
    return{
        std::max(a.x, b.x),
        std::max(a.y, b.y),
        std::max(a.z, b.z),
    };
}

b_b array_to_one(TriangleRange range){
    // auto [minimum, maximum] = m[0];
    vec3 minimum = triangle_to_b_b(range.begin[0]).min_corner;
    vec3 maximum = triangle_to_b_b(range.begin[0]).max_corner;
    while(range.begin != range.end){
        minimum = min_of_2vectors(minimum, (triangle_to_b_b(range.begin[0])).min_corner);
        maximum = max_of_2vectors(maximum, triangle_to_b_b(range.begin[0]).max_corner);
        range.begin += 1;
    }
    return{
        minimum,
        maximum,
    };
}

void swap(b_b *x,b_b *y){
    b_b a = *x;
    *x = *y;
    *y = a;
}

b_b two_one(std::variant<b_b, triangle> a, std::variant<b_b, triangle> b){
    if(std::holds_alternative<b_b>(a)){
        return{
            min_of_2vectors(std::get<b_b>(a).min_corner, std::get<b_b>(b).min_corner),
            max_of_2vectors(std::get<b_b>(a).max_corner, std::get<b_b>(b).max_corner),
        };
    }
    return{
        min_of_2vectors(triangle_to_b_b(std::get<triangle>(a)).min_corner, triangle_to_b_b(std::get<triangle>(b)).min_corner),
        max_of_2vectors(triangle_to_b_b(std::get<triangle>(a)).max_corner, triangle_to_b_b(std::get<triangle>(b)).max_corner),
    };
}

void sort_triangle(TriangleRange m, int i){
    std::sort(m.begin, m.end, [i](triangle const &tr0, triangle const &tr1){return (triangle_to_b_b(tr0).center()[i])<(triangle_to_b_b(tr1).center()[i]);});
}
triangle *sort_triangle0(TriangleRange m, int i){
    triangle *mid = m.begin + (m.end - m.begin) / 2;
    std::nth_element(m.begin, m.end, mid, [i](triangle const &tr0, triangle const &tr1){return (triangle_to_b_b(tr0).center()[i])<(triangle_to_b_b(tr1).center()[i]);});
    return mid;
}

int find_max_side(b_b box){
    float max = max_of_1(box.max_corner.x-box.min_corner.x, box.max_corner.y-box.min_corner.y, box.max_corner.z-box.min_corner.z);
    if(max == box.max_corner.x - box.min_corner.x){
        return 0;
    }
    if(max == box.max_corner.y - box.min_corner.y){
        return 1;
    }
    return 2;
}

int find_first_b_b(MaybeTwoIntersections const intersections, Ray ray, int i, b_b box){
    vec3 a,b;
    a = ray.point(intersections.tMin);
    b = ray.point(intersections.tMax);
    if(length(ray.origin - a) < length(ray.origin - b)){
        if(a[i] < box.center()[i]){
            return 1;
        }
    }
    else{
        if(b[i] > box.center()[i]){
            return 2;
        }
    }
    if(intersections.tMax == None.tMax and intersections.tMin == None.tMin){
        return 0;
    }
};

struct Intersection
{
    triangle t;
    RayTriangleIntersection i;
};

std::optional<intersection> coordinate_of_intersection(Tree tr, Ray ray, int i){
    if(std::holds_alternative<triangle>(tr->next)){
        if(happened(rayTriangleIntersection(ray, std::get<triangle>(tr->next)))){
            //triangle tr2 = std::get<triangle>(tr->next);
            intersection sect;
            sect.tr = std::get<triangle>(tr->next);
            sect.position = ray.point(rayTriangleIntersection(ray, std::get<triangle>(tr->next)).t);
            return sect;
        }
        else{
            return std::nullopt;
        }
    }
    MaybeTwoIntersections iLeft = rayboxIntersection(ray, std::get<boxnode::branches>(tr->next).left->box);
    MaybeTwoIntersections iRight = rayboxIntersection(ray, std::get<boxnode::branches>(tr->next).right->box);

    // intersections = rayboxIntersection(ray, tr->box);
    // int k = find_first_b_b(intersections, ray, i, tr->box);
    // if(k == 0){
    //     return std:: nullopt;
    // }
    // if(k == 1){
    //     return coordinate_of_intersection(std::get<boxnode::branches>(tr->next).left, ray, i);
    // }
    // if(k == 2){
    //     return coordinate_of_intersection(std::get<boxnode::branches>(tr->next).right, ray, i);

    // }
    
    if(!happened(iLeft)){
        coordinate_of_intersection(std::get<boxnode::branches>(tr->next).right, ray, i);
    }
    if(!happened(iRight)){
        coordinate_of_intersection(std::get<boxnode::branches>(tr->next).left, ray, i);
    }
    if(happened(iLeft) and happened(iRight)){
        if(iLeft.tMin < iRight.tMin){
            coordinate_of_intersection(std::get<boxnode::branches>(tr->next).left, ray, i);
            if(coordinate_of_intersection(std::get<boxnode::branches>(tr->next).left, ray, i) == std::nullopt){
                coordinate_of_intersection(std::get<boxnode::branches>(tr->next).right, ray, i);
            }
        }
        else{
            coordinate_of_intersection(std::get<boxnode::branches>(tr->next).right, ray, i);
            if(coordinate_of_intersection(std::get<boxnode::branches>(tr->next).right, ray, i) == std::nullopt){
                coordinate_of_intersection(std::get<boxnode::branches>(tr->next).left, ray, i);
            }
        }
    }
};

Tree cons(Tree left, b_b value, Tree right){
    Tree a = static_cast<Tree>(std::malloc(sizeof(boxnode)));
    a->box = value;
    a->next = boxnode::branches({left, right});
    return a;
};

Tree createTree(TriangleRange range){
    if (range.end - range.begin == 1){
        Tree node = cons(nullptr, triangle_to_b_b(*range.begin), nullptr);
        node->next = *range.begin;
        return node;
    }

    b_b box = array_to_one(range);

    triangle *mid = sort_triangle0(range, find_max_side(box));
    Tree right = createTree({range.begin, mid});
    Tree left = createTree({mid + 1, range.end});
    return cons(left, box, right);
}

// TriangleRange range_of_triangles;
// int i = find_max_side(array_to_one(range_of_triangles));
