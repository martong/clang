#!/usr/bin/env python

import generateTestCpp as g
import subprocess
import time


def run(clang, s, SM, F, SF, M):
    if SM > 0:
        for M in range(SM, 21 * SM, SM):
            av = runMF(clang, s, M, F)
            print M, av
    elif SF > 0:
        for F in range(SF, 21 * SF, SF):
            av = runMF(clang, s, M, F)
            print F, av


def runMF(clang, s, M, F):
    filename = "/tmp/friend.cpp"
    g.createFile(filename, M, F, selective=s)
    sumT = 0.0
    Max = 10
    for i in range(Max):
        sumT += runOnce(clang, filename, s)
    average = sumT/Max
    #print "average: ", average
    return average


def runOnce(clang, filename, s):
    start = time.time()
    subprocess.check_call([clang, filename, "-std=c++14", "-fsyntax-only"])
    end = time.time()
    #print(end - start)
    return (end - start)


def main():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("-pc", help="path to clang", required=True)
    parser.add_argument("-s", help="add selective friend function declarations",
                        type=bool)

    parser.add_argument("-F", help="number of friend function declarations",
                        type=int, default=0)
    parser.add_argument("-SM", help="",
                        type=int, default=0)

    parser.add_argument("-M", help="number of member access expressions",
                        type=int, default=0)
    parser.add_argument("-SF", help="",
                        type=int, default=0)

    args = parser.parse_args()

    run(args.pc, args.s, SM=args.SM, F=args.F, SF=args.SF, M=args.M)
    #createFile(args.o, args.M, args.F, args.s)

if __name__ == "__main__":
    main()
