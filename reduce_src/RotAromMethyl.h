// name: RotAromMethyl.h
// author: J. Michael Word, modified by Aram Han
// date written: 2/7/98, modified 8/13/12
// purpose: Interface for RotAromMethyl

// **************************************************************
// NOTICE: This is free software and the source code is freely
// available. You are free to redistribute or modify under the
// conditions that (1) this notice is not removed or modified
// in any way and (2) any modified versions of the program are
// also available for free.
//               ** Absolutely no Warranty **
// Copyright (C) 1999 J. Michael Word
// **************************************************************

#ifndef ROTAROMMETHYL_H
#define ROTAROMMETHYL_H 1

#include "PDBrec.h"
#include "DotSph.h"
#include "BumperPoint.h"
#include "Mover.h"
#include <vector>
#include "neighbors.h"
#include "utility.h"

class RotAromMethyl: public Mover {
public:
   RotAromMethyl(const Point3d& a, const Point3d& b,
             const double ang, const PDBrec& heavyAtom);
   virtual ~RotAromMethyl() {
//	   std::for_each(_bnded.begin(), _bnded.end(), DeleteObject());
	   for (std::vector< std::list< std::shared_ptr<PDBrec> >* >::const_iterator i = _bnded.begin(); i != _bnded.end(); ++i) {
		   std::list< std::shared_ptr<PDBrec> >* rlst = *i;
		   for (std::list< std::shared_ptr<PDBrec> >::const_iterator it = rlst->begin(); it != rlst->end(); ++it)
			   delete *it;
		   delete rlst;
	   }
	   std::for_each(_rot.begin(), _rot.end(), DeleteObject());
   }

   virtual Mover::MemoType type() { return Mover::ROTATE_METHYL; }
   virtual bool isComplete() const { return TRUE; } // rotates partial methyls
   virtual bool hasHires() const { return TRUE; }
   virtual bool canFlip() const { return FALSE; }
   virtual bool canRotate() const { return TRUE; }
   virtual int flipState() const { return 0; }
   virtual bool markFlipAtoms() { return FALSE; }
   virtual void finalize(int nBondCutoff, bool useXplorNames, bool useOldNames, bool bbModel, 
                         AtomPositions &xyz, DotSphManager& dotBucket);
   virtual int makebumpers(std::multimap<LocBlk, BumperPoint*>& bbins,
                           int n, float& maxVDWrad);
   virtual std::list<AtomDescr> getAtDescOfAllPos(float &maxVDWrad);                        
   virtual const PDBrec& exampleAtom() const { return heavyAtom(); }

   virtual int numOrientations(SearchStrategy ss=Mover::LOW_RES) const;
   virtual void limitOrientations(bool f, SearchStrategy ss=Mover::LOW_RES);
   virtual bool setOrientation(int oi, AtomPositions &xyz,
	 SearchStrategy ss=Mover::LOW_RES) {
      return setOrientation(oi, 0.0, xyz, ss);
   }
   virtual bool isDefaultO(int oi, SearchStrategy ss=Mover::LOW_RES) const;
   virtual std::string describeOrientation() const;
   virtual double determineScore(AtomPositions &xyz,
      DotSphManager& dotBucket, int nBondCutoff,
      float probeRadius, float pmag, double& penalty,
      float &bumpScore, float &hbScore, bool& hasBadBump);

   bool setOrientation(int oi, float delta, AtomPositions &xyz,
	 SearchStrategy ss=Mover::LOW_RES);
   double scoreThisAngle(AtomPositions &xyz,
      DotSphManager& dotBucket, int nBondCutoff,
      float probeRadius, float &bumpScore,
      float &hbScore, bool &hasBadBump);

   const PDBrec& heavyAtom() const { return _heavyAtom; }

   void insertHatom(const PDBrec& ha) {
       std::shared_ptr<PDBrec> temp = std::make_shared<PDBrec>();
	   *temp = ha;
	   _rot.push_front(temp); 
   }

   double orientationAngle(int oi, SearchStrategy ss=Mover::LOW_RES) const;
   double angle() const { return _angle; }

   virtual void setHydAngle(double newAng, AtomPositions &xyz);
   virtual void dropBondedFromBumpingListForPDBrec( std::list< std::shared_ptr<PDBrec> > & bumping, std::shared_ptr<PDBrec> atom, int nBondCutoff  ) const;
private:
   double orientationPenalty(float pmag) const;
   void angle(double val) { _angle = clampAngle(val); }
   void setHydAngle(PDBrec& theAtom, double oldAng, double newAng,
                        AtomPositions &xyz);
   int findAtom(std::shared_ptr<PDBrec> atom ) const;

   Point3d     _p1, _p2;   // rotation axis is through these points
   PDBrec      _heavyAtom; // hydrogen attachment point
   std::list< std::shared_ptr<PDBrec> > _rot;       // rotating hydrogen atoms
   double      _angle;
   char        _grpName[20];

   std::vector< std::list< std::shared_ptr<PDBrec> >* > _bnded; // pre-calculated bonding list

   RotAromMethyl(const RotAromMethyl& m); // copy and assign not implemented
   RotAromMethyl& operator=(const RotAromMethyl& m);
};

inline void RotAromMethyl::limitOrientations(bool, SearchStrategy) {
   /* do nothing */
}
#endif
