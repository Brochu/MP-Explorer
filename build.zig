const std = @import("std");

// Although this function looks imperative, note that its job is to
// declaratively construct a build graph that will be executed by an external
// runner.
pub fn build(b: *std.Build) void {
    const LazyPath = std.build.LazyPath;
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard optimization options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall. Here we do not
    // set a preferred release mode, allowing the user to decide how to optimize.
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "MP-Explorer",
        // In this case the main source file is merely a path, however, in more
        // complicated build scripts, this could be a generated file.
        .root_source_file = .{ .path = "src/main.cpp" },
        .target = target,
        .optimize = optimize,
    });
    exe.addIncludePath(LazyPath.relative("include/"));
    exe.addIncludePath(LazyPath.relative("include/SDL2/"));
    exe.addIncludePath(LazyPath.relative("include/imgui/"));

    const imgui_src = [_][]const u8{
        "include/imgui/imconfig.h",
        "include/imgui/imgui.h",
        "include/imgui/imgui_impl_dx12.h",
        "include/imgui/imgui_impl_win32.h",
        "include/imgui/imgui_internal.h",
        "include/imgui/imstb_rectpack.h",
        "include/imgui/imstb_textedit.h",
        "include/imgui/imstb_truetype.h",

        "src/imgui/imgui.cpp",
        "src/imgui/imgui_demo.cpp",
        "src/imgui/imgui_draw.cpp",
        "src/imgui/imgui_impl_dx12.cpp",
        "src/imgui/imgui_impl_win32.cpp",
        "src/imgui/imgui_tables.cpp",
        "src/imgui/imgui_widgets.cpp",
    };
    const flags = [_][]const u8{};
    exe.addCSourceFiles(&imgui_src, &flags);

    exe.linkLibCpp();
    exe.linkSystemLibrary("d3d12");
    exe.linkSystemLibrary("dxgi");
    //TODO: Why can't it find those?
    //exe.linkSystemLibrary("d3dcompiler");
    //exe.linkSystemLibraryNeeded("dxguid");

    exe.addObjectFile(LazyPath.relative("libs/d3dcompiler.lib"));
    exe.addObjectFile(LazyPath.relative("libs/dxguid.lib"));
    exe.addObjectFile(LazyPath.relative("libs/SDL2.lib"));
    exe.addObjectFile(LazyPath.relative("libs/SDL2main.lib"));

    // This declares intent for the executable to be installed into the
    // standard location when the user invokes the "install" step (the default
    // step when running `zig build`).
    b.installArtifact(exe);

    // This *creates* a Run step in the build graph, to be executed when another
    // step is evaluated that depends on it. The next line below will establish
    // such a dependency.
    //const run_cmd = b.addRunArtifact(exe);

    // By making the run step depend on the install step, it will be run from the
    // installation directory rather than directly from within the cache directory.
    // This is not necessary, however, if the application depends on other installed
    // files, this ensures they will be present and in the expected location.
    //run_cmd.step.dependOn(b.getInstallStep());

    // This allows the user to pass arguments to the application in the build
    // command itself, like this: `zig build run -- arg1 arg2 etc`
    //if (b.args) |args| {
    //    run_cmd.addArgs(args);
    //}

    // This creates a build step. It will be visible in the `zig build --help` menu,
    // and can be selected like this: `zig build run`
    // This will evaluate the `run` step rather than the default, which is "install".
    //const run_step = b.step("run", "Run the app");
    //run_step.dependOn(&run_cmd.step);
}
