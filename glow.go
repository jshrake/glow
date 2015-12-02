package main

import (
	"encoding/xml"
	"fmt"
	"os"
	"strings"
	"text/template"
	"time"

	"github.com/jawher/mow.cli"
)

// OpenGL XML Registry structs
type Type struct {
	Api      string `xml:"api,attr"`
	ApiEntry string `xml:"apientry"`
	Comment  string `xml:"comment,attr"`
	Name     string `xml:"name,attr"`
	RawValue string `xml:",innerxml"`
	Requires string `xml:"requires,attr"`
	TypeName string `xml:"name"`
	Value    string `xml:",chardata"`
}

type Group struct {
	Comment string `xml:"comment,attr"`
	Enums   []Enum `xml:"enum"`
	Name    string `xml:"name,attr"`
}

type Enum struct {
	Alias   string `xml:"alias,attr"`
	Comment string `xml:"comment,attr"`
	Name    string `xml:"name,attr"`
	Type    string `xml:"type,attr"`
	Value   string `xml:"value,attr"`
}

type EnumGroup struct {
	Comment   string `xml:"comment,attr"`
	End       string `xml:"end,attr"`
	Enums     []Enum `xml:"enum"`
	Group     string `xml:"group,attr"`
	Namespace string `xml:"namespace,attr"`
	Start     string `xml:"start,attr"`
	Type      string `xml:"type,attr"`
	Vendor    string `xml:"vendor,attr"`
}

type Param struct {
	Group string `xml:"group,attr"`
	Len   string `xml:"len,attr"`
	Raw   string `xml:",innerxml"`
	Name  string `xml:"name"`
	Ptype string `xml:"ptype"`
}

type Prototype struct {
	Name  string `xml:"name"`
	Value string `xml:",chardata"`
	Raw   string `xml:",innerxml"`
	Ptype string `xml:"ptype"`
}

type Command struct {
	Proto  Prototype `xml:"proto"`
	Params []Param   `xml:"param"`
}

type FeatureName struct {
	Name string `xml:"name,attr"`
}

type FeatureGroup struct {
	Comment  string        `xml:"comment,attr"`
	Profile  string        `xml:"profile,attr"`
	Commands []FeatureName `xml:"command"`
	Enums    []FeatureName `xml:"enum"`
	Types    []FeatureName `xml:"type"`
}

type Feature struct {
	Api      string         `xml:"api,attr"`
	Name     string         `xml:"name,attr"`
	Number   string         `xml:"number,attr"`
	Required []FeatureGroup `xml:"require"`
	Removed  []FeatureGroup `xml:"remove"`
}

type Extension struct {
	Name      string         `xml:"name,attr"`
	Supported string         `xml:"supported,attr"`
	Required  []FeatureGroup `xml:"require"`
}

type Registry struct {
	Types      []Type      `xml:"types>type"`
	Groups     []Group     `xml:"groups>group"`
	Enums      []EnumGroup `xml:"enums"`
	Commands   []Command   `xml:"commands>command"`
	Features   []Feature   `xml:"feature"`
	Extensions []Extension `xml:"extensions>extension"`
}

// Template structs
type TemplateCommandInfo struct {
	Name                 string
	PfnName              string
	ReturnType           string
	Args                 string
	ArgsWithLeadingComma string
	Params               string
}

type TemplateData struct {
	Debug         bool
	Version       string
	Date          string
	CommandLine   string
	Online        string
	Specification string
	Api           string
	Profile       string
	Extensions    []string
	Includes      []string
	Typedefs      []string
	VersionDefs   []string
	Enums         []Enum
	Commands      []TemplateCommandInfo
}

func main() {
	glow := cli.App("glow", "Generate a single-file C/C++ library for loading OpenGL functions")
	version := "1.0.0"
	glow.Version("v version", version)
	apiOpt := glow.StringOpt("A api", "gl", "--api=(gl | gles1 | gles2)")
	specOpt := glow.StringOpt("S spec", "4.5", "--spec=(1.0 | ... | 4.5)")
	profileOpt := glow.StringOpt("P profile", "core", "--profile=(core | compatibility)")
	debugOpt := glow.BoolOpt("D debug", true, "--debug=(false | true)")
	glow.Action = func() {
		// Parse the OpenGL xml specification from Stdin
		// Users can either cat or curl the spec as input
		input := os.Stdin
		defer input.Close()
		var registry Registry
		xmlParseErr := xml.NewDecoder(input).Decode(&registry)
		if xmlParseErr != nil {
			fmt.Println(xmlParseErr)
			cli.Exit(1)
		}
		// Build up the enums, commands, and types given the api + spec + profile
		spec := *specOpt
		api := *apiOpt
		profile := *profileOpt
		commands := make(map[string]Command)
		enums := make(map[string]Enum)
		types := make(map[string]Type)
		var versionDefs []string
		for _, f := range registry.Features {
			if f.Api == api && f.Number <= spec {
				versionDefs = append(versionDefs, f.Name)
				for _, required := range f.Required {
					inProfile := (len(required.Profile) == 0 ||
						required.Profile == "core" ||
						required.Profile == "common" ||
						required.Profile == profile)
					if inProfile {
						for _, command := range required.Commands {
							for _, commandInfo := range registry.Commands {
								if commandInfo.Proto.Name == command.Name {
									commands[command.Name] = commandInfo
									break
								}
							}
						}
						for _, enum := range required.Enums {
						EnumLoop:
							for _, enumGroup := range registry.Enums {
								for _, enumInfo := range enumGroup.Enums {
									if enumInfo.Name == enum.Name {
										enums[enum.Name] = enumInfo
										break EnumLoop
									}
								}
							}
						}
						for _, typ := range required.Types {
							for _, typInfo := range registry.Types {
								if typInfo.Name == typ.Name {
									types[typ.Name] = typInfo
									break
								}
							}
						}
					}
				}
				// Now remove items
				for _, removed := range f.Removed {
					needToRemove := removed.Profile == profile
					if needToRemove {
						for _, command := range removed.Commands {
							delete(commands, command.Name)
						}
						for _, enum := range removed.Enums {
							delete(enums, enum.Name)
						}
						for _, typ := range removed.Types {
							delete(types, typ.Name)
						}
					}
				}
			}
		}
		data := TemplateData{
			Version:       version,
			Date:          time.Now().String(),
			CommandLine:   fmt.Sprintf("glow --api=\"%s\" --spec=\"%s\" --profile=\"%s\" --debug=%t", api, spec, profile, *debugOpt),
			Online:        "https://github.com/jshrake/glow",
			Api:           api,
			Specification: spec,
			Profile:       profile,
			Debug:         *debugOpt,
			VersionDefs:   versionDefs,
		}
		// Populate the include values
		for _, include := range registry.Types {
			val := include.Value
			// NOTE(jshrake): We include the khrplatform.h header inline
			if strings.Contains(val, "#include") && include.Name != "khrplatform" {
				data.Includes = append(data.Includes, val)
			}
		}
		// Populate the type typedefs
		for _, typ := range registry.Types {
			val := typ.Value
			if !strings.Contains(val, "#include") && (len(typ.Api) == 0 || typ.Api == api) {
				// NOTE(jshrake): This assumes APIENTRYP is defined in the template
				rawVal := strings.Replace(typ.RawValue, "<apientry/>", "APIENTRYP", -1)
				rawVal = strings.Replace(rawVal, "<name>", "", -1)
				rawVal = strings.Replace(rawVal, "</name>", "", -1)
				data.Typedefs = append(data.Typedefs, rawVal)
			}
		}
		// build the data.Enums list
		for _, enum := range enums {
			data.Enums = append(data.Enums, enum)
		}
		// Build the data.Commands list
		for _, command := range commands {
			cmdName := command.Proto.Name
			pfnName := fmt.Sprintf("PFN%sPROC", strings.ToUpper(strings.Replace(cmdName, "gl_", "gl", 1)))
			// Build the signature
			// TODO(jshrake): Is building the return value this way sane?
			retVal := command.Proto.Ptype
			if len(retVal) == 0 {
				retVal = command.Proto.Value
			}
			// Build up the args and parameters strings
			// NOTE(jshrake): - args is a comma-delimited string consisting of just the argument names suitable
			//                  for function calls eg: foo(arg1, arg2, arg3), args = "arg1, arg2, arg3"
			//							  - params is a comma-delimited string consiting of the type and argument names
			//                  suitable for building signatures eg: void foo(T arg1, U arg2, V arg3),
			//                 	params = "T arg1, U arg2, V arg3"
			// TODO(jshrake): performance of string += ?
			args := ""
			params := ""
			for i, param := range command.Params {
				suffix := ", "
				if i == len(command.Params)-1 {
					suffix = ""
				}
				p := param.Raw
				// TODO(jshrake): Are these replace calls sane? Regex? Performance?
				p = strings.Replace(p, "<name>", "", -1)
				p = strings.Replace(p, "</name>", "", -1)
				p = strings.Replace(p, "<ptype>", "", -1)
				p = strings.Replace(p, "</ptype>", "", -1)
				params += fmt.Sprintf("%s%s", p, suffix)
				args += fmt.Sprintf("%s%s", param.Name, suffix)
			}
			// NOTE(jshrake): It's useful to have a version of args with a leading comma
			// or an empty string if args is empty.
			argsWithLeadingComma := ""
			if len(args) != 0 {
				argsWithLeadingComma = fmt.Sprintf(", %s", args)
			}
			data.Commands = append(data.Commands, TemplateCommandInfo{
				Name:                 cmdName,
				PfnName:              pfnName,
				ReturnType:           strings.TrimSpace(retVal),
				Args:                 args,
				ArgsWithLeadingComma: argsWithLeadingComma,
				Params:               params,
			})
		}

		// Template loading
		tmpl, tmplErr := template.ParseFiles("template/glow.h", "template/khrplatform.h")
		if tmplErr != nil {
			fmt.Println(tmplErr)
			cli.Exit(1)
		}

		// Execute the template and write to Stdout
		// Users can pipe the output to a file of their choice
		tmplExecErr := tmpl.Execute(os.Stdout, data)
		if tmplExecErr != nil {
			fmt.Println(tmplExecErr)
			cli.Exit(1)
		}
	}
	glow.Run(os.Args)
}
