{
  "targets": [
    {
      "target_name": "judger",
      "sources": [
        "./src/judger.cxx",
        "./src/exe_runner.cxx",
        "./src/proc_watcher.cxx"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}

