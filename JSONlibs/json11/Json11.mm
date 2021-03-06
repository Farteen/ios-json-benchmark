//
//  Json11.m
//  JSONlibs
//
//  Created by Loreto Parisi on 13/10/15.
//  Copyright © 2015 Mocha Code. All rights reserved.
//

#import "Json11.h"
#import "json11.hpp"
#import "CppStringAdditions.h"

@implementation Json11

- (NSString*)parse:(NSString*)json {
    
    const char* data = [json UTF8String];
    
    json11::Json parser;
    
    parser = json11::Json(data);
    
    std::string jsonString;
    
    parser.dump(jsonString);

    //printf("%s", jsonString.c_str());
    
    return [NSString stringWithstring:jsonString];

}

@end
