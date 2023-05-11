---
title: URI Templating for numeric URI encoding
abbrev: numeric-uri
docname: draft-authors-numeric-uri-scheme
category: info
submissiontype: IETF
ipr: trust200902
submissionType: info
author:
-
    fullname: Brett Reginer
    organization: Cisco
    email: TODO

normative:
informative:

--- abstract

TODO: Introduce 
* explain how this creates a set of schema for URLs, and if the a URL matches the template, then it can be compressed in a way described in this specification. 

--- middle

# Introduction

TODO Add Motivation and introdcuce the scheme
* explain many applications that use REST have URL that follow very specific pattern
* much of the entropy of these patterns can be compressed out
* allows better compression and faster processing in binary than string form 
* not meant for generic web URLs 
* example of a webex style meeting url 
* solution is schema driven compression 
* this solution is limited to applications with relatively stable URLs with low number of bits of entropy 
* give a single simple example or URL , template, and compressed binary form 



# Numeric URIs

TODO Define Scheme and Templating mechanisms
* not all URIs can be numberic URI. Generally numeric URI are designed to work this way
* Formal definition of the schema syntax and legal schemes 
* Formal defintition of how to take a URI and see if it matches a given schema
* Given URI and matching schema, explain how to compress this into an integer 

## Examples

TODO - add five schema examples that range from simple to complex and illustrate the range of functionality 

## Applicability and Limitations

TODO 

# Reference Implementation

TODO
* point at github repo 
* point to a location to keep track of well known templates 


# Security Considerations

This section needs more work
(Leave this till later)

# IANA Considerations {#iana}

* Explain uses PEN registry but does not need any changres to it. 
(Leave this till later)

--- back

# Test Vectors 

(Leave this till later)

# References

## Normative References

## Informative references

# Acknowledgments
