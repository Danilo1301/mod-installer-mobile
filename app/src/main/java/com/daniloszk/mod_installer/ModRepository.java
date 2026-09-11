package com.daniloszk.mod_installer;

import java.util.ArrayList;
import java.util.List;

public class ModRepository
{
    public String name;
    public String description;
    public String githubUrl;
    public String zipUrl;
    public List<ModVersion> versions = new ArrayList<>();
}