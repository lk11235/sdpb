//=======================================================================
// Copyright 2014-2015 David Simmons-Duffin.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================


// Currently, SDPB uses tinyxml2 to parse an input file into an XML
// tree, which is stored entirely in memory.  This tree is then
// transformed into the appropriate data structures using the parse
// functions below.  In the future, this could be made more memory
// efficient by avoiding building the XML tree in memory.

// See the manual for a description of the correct XML input format.

#include <vector>
#include "types.h"
#include "parse.h"
#include "Polynomial.h"
#include "SDP.h"
#include "tinyxml2/tinyxml2.h"

using std::vector;
using tinyxml2::XMLDocument;
using tinyxml2::XMLElement;
using boost::filesystem::path;

// parse a bunch of adjacent elements with tag `name' into a vector<T>
// using the function `parse' for each element.
template <class T>
vector<T> parseMany(const char *name, T(*parse)(XMLElement *),
                    XMLElement *elt) {
  XMLElement *e;
  vector<T> v;
  for (e = elt->FirstChildElement(name);
       e != NULL;
       e = e->NextSiblingElement(name)) {
    v.push_back(parse(e));
  }
  return v;
}

Real parseReal(XMLElement *xml) {
  return Real(xml->GetText());
}

int parseInt(XMLElement *xml) {
  return atoi(xml->GetText());
}

Vector parseVector(XMLElement *xml) {
  return parseMany("elt", parseReal, xml);
}

Matrix parseMatrix(XMLElement *xml) {
  Matrix m;
  m.rows     = parseInt(xml->FirstChildElement("rows"));
  m.cols     = parseInt(xml->FirstChildElement("cols"));
  m.elements = parseVector(xml->FirstChildElement("elements"));
  return m;
}

Polynomial parsePolynomial(XMLElement *xml) {
  Polynomial p;
  p.coefficients = parseMany("coeff", parseReal, xml);
  return p;
}

vector<Polynomial> parsePolynomialVector(XMLElement *xml) {
  return parseMany("polynomial", parsePolynomial, xml);
}

PolynomialVectorMatrix parsePolynomialVectorMatrix(XMLElement *xml) {
  PolynomialVectorMatrix m;
  m.rows           = parseInt(xml->FirstChildElement("rows"));
  m.cols           = parseInt(xml->FirstChildElement("cols"));
  m.elements       = parseMany("polynomialVector", parsePolynomialVector,
                               xml->FirstChildElement("elements"));
  m.samplePoints   = parseVector(xml->FirstChildElement("samplePoints"));
  m.sampleScalings = parseVector(xml->FirstChildElement("sampleScalings"));
  m.bilinearBasis  = parsePolynomialVector(xml->FirstChildElement("bilinearBasis"));
  return m;
}

SDP parseBootstrapSDP(XMLElement *xml) {
  return bootstrapSDP(parseVector(xml->FirstChildElement("objective")),
                      parseMany("polynomialVectorMatrix",
                                parsePolynomialVectorMatrix,
                                xml->FirstChildElement("polynomialVectorMatrices")));
}

SDP readBootstrapSDP(const path sdpFile) {
  XMLDocument doc;
  doc.LoadFile(sdpFile.string().c_str());
  return parseBootstrapSDP(doc.FirstChildElement("sdp"));
}
