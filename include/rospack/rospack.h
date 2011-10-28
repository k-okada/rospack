/*
 * Copyright (C) 2008, Willow Garage, Inc.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the names of Stanford University or Willow Garage, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef ROSPACK_ROSPACK_H
#define ROSPACK_ROSPACK_H

#include <boost/tr1/unordered_set.hpp>
#include <boost/tr1/unordered_map.hpp>
#include <string>
#include <map>
#include <vector>
#include <stdexcept>

#ifdef ROSPACK_API_BACKCOMPAT_V1
  #include "rospack/rospack_backcompat.h"
#endif


namespace rospack
{

static const char* ROSPACK_NAME = "rospack";
static const char* ROSSTACK_NAME = "rosstack";
static const char* MANIFEST_TAG_PACKAGE = "package";
static const char* MANIFEST_TAG_STACK = "stack";

typedef enum
{
  CRAWL_UP,
  CRAWL_DOWN
} crawl_direction_t;

typedef enum 
{
  POSTORDER,
  PREORDER
} traversal_order_t;

class Stackage;
class DirectoryCrawlRecord;

class Rosstackage
{
  private:
    std::string manifest_name_;
    std::string cache_name_;
    crawl_direction_t crawl_dir_;

    bool crawled_;
    std::string name_;
    std::string tag_;
    bool quiet_;
    std::vector<std::string> search_paths_;
    std::tr1::unordered_set<std::string> dups_;
    std::tr1::unordered_map<std::string, Stackage*> stackages_;
    void log(const std::string& level, const std::string& msg, bool append_errno);
    void addStackage(const std::string& path);
    void crawlDetail(const std::string& path,
                     bool force,
                     int depth,
                     bool collect_profile_data,
                     std::vector<DirectoryCrawlRecord*>& profile_data,
                     std::tr1::unordered_set<std::string>& profile_hash);
    bool dependsOnDetail(const std::string& name, bool direct,
                         std::vector<Stackage*>& deps);
    bool depsDetail(const std::string& name, bool direct,
                    std::vector<Stackage*>& deps);
    bool isStackage(const std::string& path);
    void loadManifest(Stackage* stackage);
    void computeDeps(Stackage* stackage, bool ignore_errors=false);
    void gatherDeps(Stackage* stackage, bool direct, 
                    traversal_order_t order,
                    std::vector<Stackage*>& deps);
    void gatherDepsFull(Stackage* stackage, bool direct, 
                        traversal_order_t order, int depth, 
                        std::tr1::unordered_set<Stackage*>& deps_hash,
                        std::vector<Stackage*>& deps,
                        bool get_indented_deps,
                        std::vector<std::string>& indented_deps);
    std::string getCachePath();
    bool readCache();
    void writeCache();
    bool validateCache();
    bool expandExportString(Stackage* stackage,
                            const std::string& instring,
                            std::string& outstring);

  protected:

  public:
    Rosstackage(const std::string& manifest_name,
                const std::string& cache_name,
                crawl_direction_t crawl_dir,
                const std::string& name,
                const std::string& tag);
    virtual ~Rosstackage();

    virtual const char* usage() = 0;
    void crawl(const std::vector<std::string>& search_path, bool force);
    bool inStackage(std::string& name);
    void setQuiet(bool quiet);
    const std::string& getName() {return name_;}
    bool getSearchPathFromEnv(std::vector<std::string>& sp);
    bool find(const std::string& name, std::string& path); 
    bool contents(const std::string& name, std::vector<std::string>& packages);
    bool contains(const std::string& name, 
                  std::string& stack,
                  std::string& path);
    void list(std::vector<std::pair<std::string, std::string> >& list);
    void listDuplicates(std::vector<std::string>& dups);
    bool deps(const std::string& name, bool direct, std::vector<std::string>& deps);
    bool dependsOn(const std::string& name, bool direct,
                   std::vector<std::string>& deps);
    bool depsManifests(const std::string& name, bool direct, 
                       std::vector<std::string>& manifests);
    bool depsMsgSrv(const std::string& name, bool direct, 
                    std::vector<std::string>& gens);
    bool depsIndent(const std::string& name, bool direct,
                    std::vector<std::string>& deps);
    bool rosdeps(const std::string& name, bool direct,
                 std::vector<std::string>& rosdeps);
    bool vcs(const std::string& name, bool direct, 
             std::vector<std::string>& vcs);
    bool exports(const std::string& name, const std::string& lang,
                 const std::string& attrib, bool deps_only,
                 std::vector<std::string>& flags);
    bool plugins(const std::string& name, const std::string& attrib, 
                 const std::string& top,
                 std::vector<std::string>& flags);
    bool profile(const std::vector<std::string>& search_path,
                 bool zombie_only,
                 int length,
                 std::vector<std::string>& dirs);
    // Simple console output helpers
    void log_warn(const std::string& msg,
                  bool append_errno = false);
    void log_error(const std::string& msg,
                   bool append_errno = false);

};

class Rospack : public Rosstackage
{
  public:
    Rospack();
    virtual const char* usage();
};

class Rosstack : public Rosstackage
{
  public:
    Rosstack();
    virtual const char* usage();
};

void deduplicate_tokens(const std::string& instring, 
                        bool last,
                        std::string& outstring);
bool rospack_run(int argc, char** argv, 
                 rospack::Rosstackage& rp, 
                 std::string& output);
} // namespace rospack


#endif
