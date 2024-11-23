//
// Created by dpeter99 on 2023.09.10..
//
#include "ecs.exp.h"

struct A {};
struct B {};
struct C {};


int main() {

    EntityManager em;
    auto& a = em.GetArchetype({
        GetTypeId<A>().id,
        GetTypeId<C>(TypeFlags::Flag).id,
        GetTypeId<B>().id,
    });

    PrintArchetype(a);

    return 0;
}
