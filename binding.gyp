{
  "targets": [
    {
      "target_name": "judger",
      "sources": [
        "./src/judger.cc",
        "./src/runner.cc",
        "./src/common.cc"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}