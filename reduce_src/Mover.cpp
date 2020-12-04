// name: Mover.C
// author: J. Michael Word
// date written: 2/7/98
// purpose: utility routines for bonding and geometric relationships

// **************************************************************
// NOTICE: This is free software and the source code is freely
// available. You are free to redistribute or modify under the
// conditions that (1) this notice is not removed or modified
// in any way and (2) any modified versions of the program are
// also available for free.
//               ** Absolutely no Warranty **
// Copyright (C) 1999 J. Michael Word
// **************************************************************

#include "Mover.h"

// ResBlk.h is included to define sameres()
#include "ResBlk.h"

/*SJ what does f!=0 inside [] mean? f!=0 is a check that returns a 0 if f is equal to  0 and returns a 1 if f is any other number. So if f that is passed to these two functions is 0, then the first element of the array is changed/referenced, if f is any number other than a 0, the second element of the array is changed/referenced */
void Mover::resetFlipMaxScore(int f) {
     _flipMaxScore[f!=0] = LowestMoverScore;
   _flipMaxBadBump[f!=0] = FALSE;
}

void Mover::trackFlipMaxScore(int f, double val, bool hasBadBump) {
   if ( _flipMaxScore[f!=0] < val) {
        _flipMaxScore[f!=0] = val;
      _flipMaxBadBump[f!=0] = hasBadBump;
   }
}

void bondedList(const PDBrec& a, std::list< std::shared_ptr<PDBrec> >& nearby, int nbnds,
				std::list< std::shared_ptr<PDBrec> >& atmList, std::list< std::shared_ptr<PDBrec> >* bondedAtoms) {
	resetMarks(nearby);
	countBonds(a, nearby, 1, nbnds, atmList); // up to nbnds bonds away

	std::shared_ptr<PDBrec> rec;
	for (std::list< std::shared_ptr<PDBrec> >::const_iterator it = nearby.begin(); it != nearby.end(); ++it) {
		rec = *it;
		if (rec->valid()) {
		  if (rec->mark() >= 1 && rec->mark() <= nbnds) {
			  std::shared_ptr<PDBrec> temp = std::make_shared<PDBrec>(*rec);
			bondedAtoms->push_front(temp);
		  }
        }
	}
}

void countBonds(const PDBrec& src, const std::list< std::shared_ptr<PDBrec> >& nearby,
				int distcount, int maxcnt, std::list< std::shared_ptr<PDBrec> >& atmList) {

	std::list< std::shared_ptr<PDBrec> > newlyMarked;
	std::list< std::shared_ptr<PDBrec> > remainder;

	std::shared_ptr<PDBrec> rec;
	for (std::list< std::shared_ptr<PDBrec> >::const_iterator targ = nearby.begin(); targ != nearby.end(); ++targ) {
		rec = *targ;
		if (rec->valid()) {
		  if ( (rec->mark() < 1 || rec->mark() > distcount)
			&& ! diffAltLoc(src, *rec) ) {

			bool isnear   = withinCovalentDist(src, *rec,  0.2);
			bool tooclose = withinCovalentDist(src, *rec, -0.5);

			if (isnear && ! (tooclose || impossibleCovalent(src, *rec, atmList))) {
				rec->mark(distcount);
				newlyMarked.push_front(rec);
			}
			else {
				remainder.push_front(rec);
			}
		  }
		}
	}
	if (distcount < maxcnt) {
		for(std::list< std::shared_ptr<PDBrec> >::const_iterator it = newlyMarked.begin(); it != newlyMarked.end(); ++it) {
			countBonds(**it, remainder, distcount+1, maxcnt, atmList);
		}
	}
}

// initialize the markers we use to determine bonding patterns
void resetMarks(std::list< std::shared_ptr<PDBrec> >& lst) {
	for (std::list< std::shared_ptr<PDBrec> >::const_iterator it = lst.begin(); it != lst.end(); ++it) {
		(*it)->mark(0);
	}
}

bool visableAltConf(const PDBrec& a, bool onlyA) {
   const char aalt = a.alt();
   bool returnvalue = (aalt == ' ' || aalt == 'A' || aalt == 'a' || !onlyA);
	//std::cerr << "visiableAltConf: " << a.recName() << " "  << returnvalue << std::endl;
	return returnvalue;
}

bool interactingConfs(const PDBrec& a, const PDBrec& b, bool onlyA) {
   return ( (!visableAltConf(a, onlyA)) || (!visableAltConf(b, onlyA)) )
      ? FALSE
      : (! diffAltLoc(a, b));
}

// are a and b alternative locations for the same residue?
bool diffAltLoc(const PDBrec& a, const PDBrec& b) {
   //return a.resno() == b.resno() && a.insCode() == b.insCode()
   return a.insCode() == b.insCode()
       && strcmp(a.chain(), b.chain()) == 0
       && a.alt() != b.alt() && a.alt() != ' ' && b.alt() != ' ';
}

int withinCovalentDist(const PDBrec& p, const PDBrec& q, double offset) {
   double lim = p.covRad() + q.covRad() + offset;

   return distanceSquared(p.loc(), q.loc()) <= (lim*lim);
}

bool impossibleCovalent(const PDBrec& src, const PDBrec& targ, std::list< std::shared_ptr<PDBrec> >& atmList) {
   if (src.isHydrogen() && targ.isHydrogen()) { return TRUE; }

   else if (src.isHydrogen() || targ.isHydrogen()) {
      bool srcInList = FALSE, targInList = FALSE;
      for(std::list< std::shared_ptr<PDBrec> >::const_iterator it = atmList.begin(); it != atmList.end(); ++it) {
	      if (**it == src)  {srcInList  = TRUE; }
	 else if (**it == targ) {targInList = TRUE; }
      }

      if (srcInList && targInList) { return FALSE; }
      else if (srcInList && src.isHydrogen())   { return TRUE; }
      else if (targInList && targ.isHydrogen()) { return TRUE; }
   }

   return (! sameres(src, targ)) && (src.isHydrogen() || targ.isHydrogen());
}

bool foundInList(const PDBrec& a, const std::list< std::shared_ptr<PDBrec> >& lst) {
	for (std::list< std::shared_ptr<PDBrec> >::const_iterator it = lst.begin(); it != lst.end(); ++it) {
		if (a == **it) { return TRUE; }
	}
	return FALSE;
}

// what is the gap between the VDW radii of atom a at point aloc
// and atom b at point bloc?

double vdwGap(const PDBrec& a, const Point3d& aloc,
              const PDBrec& b, const Point3d& bloc) {
   double lim = a.vdwRad() + b.vdwRad();

   return distance2(aloc, bloc) - lim;
}

// functions used to restrict annular rings of good dots around clashes

bool annularDots(const Point3d& dot, const PDBrec& src, const PDBrec& targ, float probeRadius) {
   return dot2srcCenter(dot, src, targ) >
      kissEdge2bullsEye(src.vdwRad(), targ.vdwRad(), probeRadius);
}

double dot2srcCenter(const Point3d& dot, const PDBrec& src, const PDBrec& targ) {

   const Point3d src2targVec = (targ.loc() - src.loc()).scaleTo(src.vdwRad());
   const Point3d srcSurfacePoint = src2targVec + src.loc();
   return distance2(dot, srcSurfacePoint);
}

double kissEdge2bullsEye(float ra, float rb, float rp) {
   return 2.0*ra*sqrt(rb*rp/((ra+rb)*(ra+rp)));
}

// force sorting of Seq<MoverPtr> by descr()
// by duplicating some merge-sort code

void mpSeqSort(std::list<MoverPtr>& x) {
	x.reverse();
}

/*
std::list<MoverPtr> mpSeqMerge(const std::list<MoverPtr>& x_list, const std::list<MoverPtr>& y_list) {
	std::list<MoverPtr> r;
	std::list<MoverPtr>::const_iterator x = x_list.begin();
	std::list<MoverPtr>::const_iterator y = y_list.begin();

   while ((x != x_list.end()) && (y != y_list.end())) {
      if ( (*x) && (*y) && ((*x)->descr() < (*y)->descr())) {
         r.push_front(*x);
         ++x;
      } else {
         r.push_front(*y);
         ++y;
      }
   }
   while (x != x_list.end()) {
      r.push_front(*x);
      ++x;
   }
   while (y != y_list.end()) {
      r.push_front(*y);
      ++y;
   }
   r.reverse();
   return r;
}

void mpSeqSplit(std::list<MoverPtr> x_list, std::list<MoverPtr>& y, std::list<MoverPtr>& z) {
	std::list<MoverPtr>::const_iterator x = x_list.begin();
	while(x != x_list.end()) {
		y.push_front(*x);
		++x;
		if (x != x_list.end()) {
			z.push_front(*x);
			++x;
		}
	}
}

std::list<MoverPtr> mpSeqSort(const std::list<MoverPtr>& x) {
   if (x.empty() || (x.size() == 1)) { return x; }
   std::list<MoverPtr> p, q;
   mpSeqSplit(x, p, q);
   return mpSeqMerge(mpSeqSort(p), mpSeqSort(q));
}

bool mpLess(const MoverPtr& x, const MoverPtr& y) {
	return x->descr() < y->descr();
}
*/
