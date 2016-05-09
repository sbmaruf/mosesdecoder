/*
 * KENLM.h
 *
 *  Created on: 4 Nov 2015
 *      Author: hieu
 */
#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <pthread.h>

#include "../FF/StatefulFeatureFunction.h"
#include "lm/model.hh"
#include "../legacy/Factor.h"
#include "../legacy/Util2.h"
#include "../Word.h"

namespace Moses2
{

class Word;

class KENLMBatch: public StatefulFeatureFunction
{
public:
  KENLMBatch(size_t startInd, const std::string &line);

  virtual ~KENLMBatch();

  virtual void Load(System &system);

  void SetParameter(const std::string& key,
      const std::string& value);

  virtual FFState* BlankState(MemPool &pool) const;

  //! return the state associated with the empty hypothesis for a given sentence
  virtual void EmptyHypothesisState(FFState &state, const ManagerBase &mgr,
      const InputType &input, const Hypothesis &hypo) const;

  virtual void
  EvaluateInIsolation(MemPool &pool, const System &system, const Phrase<Moses2::Word> &source,
      const TargetPhrase<Moses2::Word> &targetPhrase, Scores &scores,
      SCORE *estimatedScore) const;

  virtual void
  EvaluateInIsolation(MemPool &pool, const System &system, const Phrase<SCFG::Word> &source,
      const TargetPhrase<SCFG::Word> &targetPhrase, Scores &scores,
      SCORE *estimatedScore) const;

  virtual void EvaluateWhenApplied(const ManagerBase &mgr,
      const Hypothesis &hypo, const FFState &prevState, Scores &scores,
      FFState &state) const;

  virtual void EvaluateWhenAppliedBatch(
      const std::vector<Hypothesis*> &batch) const;

protected:
  std::string m_path;
  FactorType m_factorType;
  util::LoadMethod m_load_method;
  const Factor *m_bos;
  const Factor *m_eos;

  typedef lm::ngram::ProbingModel Model;
  boost::shared_ptr<Model> m_ngram;

  void CalcScore(const Phrase<Moses2::Word> &phrase, float &fullScore, float &ngramScore,
      std::size_t &oovCount) const;

  inline lm::WordIndex TranslateID(const Word &word) const
  {
    std::size_t factor = word[m_factorType]->GetId();
    return (factor >= m_lmIdLookup.size() ? 0 : m_lmIdLookup[factor]);
  }
  // Convert last words of hypothesis into vocab ids, returning an end pointer.
  lm::WordIndex *LastIDs(const Hypothesis &hypo, lm::WordIndex *indices) const;

  std::vector<lm::WordIndex> m_lmIdLookup;

  // batch
  typedef std::vector<Hypothesis*> Batch;
  mutable std::vector<const Batch*> m_batches;
  mutable size_t m_numHypos;

  mutable boost::shared_mutex m_accessLock;

  mutable boost::mutex m_mutex;
  mutable boost::condition_variable m_threadNeeded;

  void EvaluateWhenAppliedBatch() const;

};

}
