#!/usr/bin/env python


def createFile(filename, M, F, selective):

    MemberAcc = "[&x](){ x.mem_fun(); }();"
    MemberAccesses = ""
    for i in range(M):
        MemberAccesses += MemberAcc

    FriendDefs = ""
    for i in range(F):
        SelectiveAttr = ""
        if selective:
            SelectiveAttr = "__attribute__((friend_for(&X::x)))"
        FriendDefs += "%s friend void friend_fun%i() {}" % (SelectiveAttr, i)

    Class = (
             "class X {"
             "  int x;"
             "  void mem_fun() {}"
             "  %s"
             "  friend void caller(X& x);"
             "};") % FriendDefs
    Funct = (
             "void caller(X& x) {"
             "  %s"
             "}") % MemberAccesses
    Content = Class + Funct
    #print Content
    with open(filename, 'w') as File:
        File.write(Content)


def main():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-o",
        help="path to outputfile",
     default="/tmp/friend.cpp")
    parser.add_argument("-M", help="number of member access expressions",
                        type=int, required=True)
    parser.add_argument("-F", help="number of friend function declarations",
                        type=int, required=True)
    parser.add_argument("-s", help="add selective friend function declarations",
                        type=bool)
    args = parser.parse_args()

    createFile(args.o, args.M, args.F, args.s)

if __name__ == "__main__":
    main()
